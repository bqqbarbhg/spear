#include "Sprite.h"

#include "ContentFile.h"
#include "RectPacker.h"
#include "Image.h"

#include "ext/stb/stb_image.h"
#include "ext/stb/stb_image_resize.h"
#include "ext/sokol/sokol_gfx.h"

#include "sf/Vector.h"

namespace sp {

// TODO: Some of these could be dynamic
static const uint32_t AtlasDeleteQueueFrames = 4;
static const uint32_t MinAtlasExtent = 128;
static const uint32_t MaxAtlasExtent = 1024;
static const uint32_t AtlasLevels = 3;
static const uint32_t TargetPages = 4;
static const uint32_t MaxSpriteExtent = 248;

struct SpriteImp;
struct AtlasImp;

struct AtlasToDelete
{
	AtlasImp *atlas;
	uint32_t frameIndex;
};

struct SpriteList
{
	sf::Mutex mutex;
	uint32_t frameIndex = 0;
	sf::Array<AtlasToDelete> atlasesToDelete;
	sf::Array<AtlasImp*> atlases;
	sf::Array<SpriteImp*> residencyQueue;
	RectPacker packer;
};

SpriteList g_spriteList;

struct AtlasImp : Atlas
{
	sf::Array<SpriteImp*> sprites;
	sg_image image;
};

struct SpriteImp : Sprite
{
	virtual void startLoadingImp() final;
	virtual void unloadImp() final;

	void *data;
	sf::SmallArray<SpriteResidency, 2> residency;
	bool inResidencyQueue = false;
	uint32_t flushesThisFrame = 0;
};

const AssetType Sprite::AssetType = { "Sprite", sizeof(SpriteImp),
	[](Asset *a) { new ((SpriteImp*)a) SpriteImp(); }
};

static void loadImp(void *user, const ContentFile &file)
{
	SpriteImp *imp = (SpriteImp*)user;
	if (!file.isValid()) {
		imp->failLoadingImp();
		return;
	}

	int width, height;
	stbi_uc *pixels = stbi_load_from_memory((const stbi_uc*)file.data, (int)file.size, &width, &height, NULL, 4);
	if (!pixels) return;

	if (width > MaxSpriteExtent || height > MaxSpriteExtent) {
		uint32_t srcWidth = width, srcHeight = height;
		double aspect = (double)width / (double)height;
		if (width >= height) {
			width = MaxSpriteExtent;
			height = (uint32_t)(MaxSpriteExtent / aspect);
		} else {
			height = MaxSpriteExtent;
			width = (uint32_t)(MaxSpriteExtent * aspect);
		}

		stbi_uc *newPixels = (stbi_uc*)malloc(width * height * 4);
		stbir_resize_uint8_srgb(
			pixels, srcWidth, srcHeight, 0,
			newPixels, width, height, 0,
			4, 3, 0);

		free(pixels);
		pixels = newPixels;
	}

	imp->data = pixels;
	imp->width = width;
	imp->height = height;

	imp->finishLoadingImp();
}

void SpriteImp::startLoadingImp()
{
	ContentFile::load(name, &loadImp, this);
}

void SpriteImp::unloadImp()
{
	free(data);

	// Remove the sprite from all the atlases
	SpriteList &list = g_spriteList;
	sf::MutexGuard mg(g_spriteList.mutex);

	for (SpriteResidency &res : residency) {
		AtlasImp *atlas = (AtlasImp*)res.atlas;
		for (SpriteImp *&sprite : atlas->sprites) {
			if (sprite == this) {
				atlas->sprites.removeSwapPtr(&sprite);
				break;
			}
		}
	}
}

uint32_t Atlas::getTexture()
{
	AtlasImp *imp = (AtlasImp*)this;
	return imp->image.id;
}

void Atlas::getAtlases(sf::Array<Atlas*> &atlases)
{
	sf::MutexGuard mg(g_spriteList.mutex);
	SpriteList &list = g_spriteList;
	for (AtlasImp *atlas : list.atlases) {
		atlases.push(atlas);
	}
}

void Sprite::willBeRendered()
{
	SpriteImp *imp = (SpriteImp*)this;
	sf_assert(isLoaded());

	// If the sprite is going to be rendered but isn't resident in any
	// atlas we need to add it to one before rendering
	if (imp->residency.size == 0) {
		sf::MutexGuard mg(g_spriteList.mutex);
		if (!imp->inResidencyQueue) {
			imp->inResidencyQueue = true;
			g_spriteList.residencyQueue.push(imp);
		}
	}
}

void Sprite::getResidency(sf::Array<SpriteResidency> &dst) const
{
	SpriteImp *imp = (SpriteImp*)this;
	sf_assert(isLoaded());

	dst.clear();
	dst.push(imp->residency.data, imp->residency.size);
}

bool Sprite::isResident() const
{
	SpriteImp *imp = (SpriteImp*)this;
	sf_assert(isLoaded());
	return imp->residency.size > 0;
}

void Sprite::update()
{
	sf::MutexGuard mg(g_spriteList.mutex);
	SpriteList &list = g_spriteList;
	list.frameIndex++;

	for (uint32_t i = 0; i < list.atlasesToDelete.size; i++) {
		if (list.frameIndex - list.atlasesToDelete[i].frameIndex < AtlasDeleteQueueFrames) continue;
		AtlasImp *atlas = list.atlasesToDelete[i].atlas;

		sg_destroy_image(atlas->image);
		delete atlas;

		list.atlasesToDelete.removeSwap(i--);
	}
}

void Sprite::updateAtlasesForRendering()
{
	sf::MutexGuard mg(g_spriteList.mutex);
	SpriteList &list = g_spriteList;
	if (list.residencyQueue.size == 0) {
		// All sprites are resident, nothing to do!
		// TODO: Collect/optimize empty atlases?
		return;
	}

	// Estimate how many atlases we need
	uint32_t area = 0;
	for (SpriteImp *sprite : list.residencyQueue) {
		area += sprite->width * sprite->height;
	}

	uint32_t atlasArea = MaxAtlasExtent * MaxAtlasExtent;
	uint32_t numPages = (area + atlasArea - 1) / atlasArea;

	// Remove pages until we get to target
	while (list.atlases.size > 0 && list.atlases.size + numPages > TargetPages) {
		AtlasImp **bestAtlas = nullptr;
		uint32_t bestNumRelocate = UINT32_MAX;

		// Count how many sprites we would need to relocate
		// if we were to remove this atlas page
		for (AtlasImp *&atlas : list.atlases) {
			uint32_t numRelocate = 0;
			for (SpriteImp *sprite : atlas->sprites) {
				if (sprite->residency.size == 1) {
					numRelocate++;
				}
			}
			if (numRelocate < bestNumRelocate) {
				bestAtlas = &atlas;
				bestNumRelocate = numRelocate;
			}
		}

		// Remove the best atlas and queue the sprites
		// for relocation
		sf_assert(bestAtlas != nullptr);
		AtlasImp *atlas = *bestAtlas;
		for (uint32_t i = 0; i < atlas->sprites.size; i++) {
			SpriteImp *sprite = atlas->sprites[i];

			if (sprite->residency.size > 1) {
				// Still resident in some atlas, remove the deleted
				// atlas only from the residency list
				for (SpriteResidency &res : sprite->residency) {
					if (res.atlas == atlas) {
						sprite->residency.removeOrderedPtr(&res);
						break;
					}
				}
				continue;
			}

			sprite->residency.clear();
			sf_assert(sprite->inResidencyQueue == false);
			sprite->inResidencyQueue = true;
			list.residencyQueue.push(sprite);
			area += sprite->width + sprite->height;
		}

		// Remove the atlas and queue it for deletion
		list.atlases.removeSwapPtr(bestAtlas);
		atlas->sprites.clear();
		list.atlasesToDelete.push({ atlas, list.frameIndex });

		// Update new page estimate
		numPages = (area + atlasArea - 1) / atlasArea;
	}

	// Sort the sprites that will be inserted by size

	qsort(list.residencyQueue.data, list.residencyQueue.size, sizeof(SpriteImp*),
	[](const void *va, const void *vb) -> int {
		SpriteImp *a = (SpriteImp*)va, *b = (SpriteImp*)vb;
		uint32_t as = sf::max(a->width, a->height) * 32 + sf::min(a->width, a->height);
		uint32_t bs = sf::max(b->width, b->height) * 32 + sf::min(b->width, b->height);
		if (as < bs) return -1;
		if (as > bs) return +1;
		return 0;
	});

	while (list.residencyQueue.size > 0) {
		AtlasImp *atlas = new AtlasImp();
		list.atlases.push(atlas);

		uint32_t pad = 1 << AtlasLevels;

		uint32_t extent = MinAtlasExtent;

		// Try to pack to smaller atlases first
		// NOTE: We can skip attempting to pack the largest
		// size as it will be done again anyway
		while (extent < MaxAtlasExtent) {
			list.packer.reset(extent, extent);
			bool failed = false;
			for (SpriteImp *sprite : list.residencyQueue) {
				uint32_t x, y;

				// Pad and align the sprites
				uint32_t width = sprite->width + pad;
				uint32_t height = sprite->height + pad;
				width += (uint32_t)-(int32_t)width & (pad - 1);
				height += (uint32_t)-(int32_t)height & (pad - 1);

				if (!list.packer.pack(width, height, x, y)) {
					failed = true;
					break;
				}
			}

			if (!failed) break;
			extent *= 2;
		}

		atlas->width = extent;
		atlas->height = extent;

		list.packer.reset(extent, extent);
		MipImage image(extent, extent);
		image.levels[0].clear();

		SpriteImp **dst = list.residencyQueue.data;
		for (SpriteImp *sprite : list.residencyQueue) {
			uint32_t x, y;

			// Pad and align the sprites
			uint32_t width = sprite->width + pad;
			uint32_t height = sprite->height + pad;
			width += (uint32_t)-(int32_t)width & (pad - 1);
			height += (uint32_t)-(int32_t)height & (pad - 1);

			if (list.packer.pack(width, height, x, y)) {
				// Register the sprite to the atlas
				SpriteResidency &res = sprite->residency.push();
				res.atlas = atlas;
				res.x = x;
				res.y = y;
				atlas->sprites.push(sprite);
				sprite->inResidencyQueue = false;

				// TODO: Write insert()
				if (sprite->residency.size > 1) {
					qsort(sprite->residency.data, sprite->residency.size, sizeof(SpriteResidency),
					[](const void *va, const void *vb) -> int {
						SpriteResidency *a = (SpriteResidency*)va, *b = (SpriteResidency*)vb;
						if (a->atlas < b->atlas) return -1;
						if (a->atlas > b->atlas) return +1;
						return 0;
					});
				}

				// Blit the image content
				image.levels[0].blit(x, y, sprite->data, sprite->width, sprite->height);
			} else {
				// Did not fit: Keep in the list
				*dst++ = sprite;
			}
		}

		// Generate mips
		image.calculateMips();

		// Initialize the GPU texture
		{
			sg_image_desc desc = { };
			desc.type = SG_IMAGETYPE_2D;
			desc.width = extent;
			desc.height = extent;
			desc.num_mipmaps = image.levels.size;
			desc.usage = SG_USAGE_IMMUTABLE;
			desc.pixel_format = SG_PIXELFORMAT_RGBA8;
			desc.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
			desc.mag_filter = SG_FILTER_LINEAR;
			desc.max_lod = (float)(AtlasLevels - 1);
			desc.max_anisotropy = 0;
			desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
			desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
			desc.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
			desc.label = "Atlas Page";
			for (uint32_t i = 0; i < image.levels.size; i++) {
				desc.content.subimage[0][i].ptr = image.levels[i].data;
				desc.content.subimage[0][i].size = image.levels[i].byteSize();
			}
			atlas->image = sg_make_image(&desc);
		}

		// Shrink `residencyQueue`
		list.residencyQueue.resize(dst - list.residencyQueue.data);
	}
}

}
