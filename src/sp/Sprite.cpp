#include "Sprite.h"

#include "ContentFile.h"
#include "RectPacker.h"
#include "Image.h"

#include "ext/stb/stb_image.h"
#include "ext/stb/stb_image_resize.h"
#include "ext/sokol/sokol_config.h"
#include "ext/sokol/sokol_gfx.h"
#include "sf/ext/mx/mx_platform.h"

#include "sf/Vector.h"

namespace sp {

// TODO: Some of these could be dynamic
static const uint32_t AtlasDeleteQueueFrames = 4;
static const uint32_t MinAtlasExtent = 64;
static const uint32_t MaxAtlasExtent = 1024;
static const uint32_t AtlasLevels = 3;
static const uint32_t AtlasPadding = 1 << AtlasLevels;
static const uint32_t MaxSpriteExtent = 256 - AtlasPadding;
static const uint32_t FramesBetweenReassign = 60;

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
	RectPacker packer;
};

SpriteList g_spriteList;

struct AtlasImp : Atlas
{
	sf::Array<SpriteImp*> sprites;
	sg_image image;

	uint32_t numBrokenBatches = 0;
	uint32_t frameCreated = 0;
};

struct SpriteImp : Sprite
{
	virtual void startLoadingImp() final;
	virtual void unloadImp() final;

	uint32_t prevX = 0, prevY = 0;
	uint32_t flushesThisFrame = 0;
};

const AssetType Sprite::AssetType = { "Sprite", sizeof(SpriteImp),
	[](Asset *a) { new ((SpriteImp*)a) SpriteImp(); }
};

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static uint32_t roundToPow2(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

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

	imp->width = width;
	imp->height = height;

	// Create a single sprite atlas

	uint32_t atlasWidth = width + AtlasPadding;
	uint32_t atlasHeight = height + AtlasPadding;

	// TODO: Skip this for non-WebGL1 backends?
	atlasWidth = roundToPow2(atlasWidth);
	atlasHeight = roundToPow2(atlasHeight);
	if (atlasWidth < MinAtlasExtent) atlasWidth = MinAtlasExtent;
	if (atlasHeight < MinAtlasExtent) atlasHeight = MinAtlasExtent;

	MipImage image(atlasWidth, atlasHeight);

	image.levels[0].blit(0, 0, width, height, pixels);
	image.levels[0].clear(width, 0, atlasWidth - width, atlasHeight);
	image.levels[0].clear(0, height, width, atlasHeight - height);
	image.calculateMips();

	AtlasImp *atlas = new AtlasImp();
	atlas->width = atlasWidth;
	atlas->height = atlasHeight;

	// Initialize the GPU texture
	{
		sg_image_desc desc = { };
		desc.type = SG_IMAGETYPE_2D;
		desc.width = atlasWidth;
		desc.height = atlasHeight;
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
		desc.label = "Single Atlas";
		for (uint32_t i = 0; i < image.levels.size; i++) {
			desc.content.subimage[0][i].ptr = image.levels[i].data;
			desc.content.subimage[0][i].size = image.levels[i].byteSize();
		}
		atlas->image = sg_make_image(&desc);
	}

	free(pixels);

	atlas->sprites.push(imp);
	imp->atlas = atlas;

	{
		SpriteList &list = g_spriteList;
		sf::MutexGuard mg(list.mutex);
		list.atlases.push(atlas);
		atlas->frameCreated = list.frameIndex;
	}

	imp->finishLoadingImp();
}

void SpriteImp::startLoadingImp()
{
	ContentFile::load(name, &loadImp, this);
}

void SpriteImp::unloadImp()
{
	// Remove the sprite from all the atlases
	SpriteList &list = g_spriteList;
	sf::MutexGuard mg(g_spriteList.mutex);

	AtlasImp *atlasImp = (AtlasImp*)atlas;
	sf::findRemoveSwap(atlasImp->sprites, this);

	if (atlasImp->sprites.size == 0) {
		sf::findRemoveSwap(list.atlases, atlasImp);
		list.atlasesToDelete.push({ atlasImp, list.frameIndex });
	}
}

void Atlas::brokeBatch()
{
	AtlasImp *imp = (AtlasImp*)this;
	mxa_inc32_nf(&imp->numBrokenBatches);
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

static float scoreAtlasForReassigning(SpriteList &list, AtlasImp *atlas)
{
	uint32_t brokenBatches = mxa_load32_nf(&atlas->numBrokenBatches);
	float brokenBatchesPerFrame = (float)brokenBatches / (float)FramesBetweenReassign;

	uint32_t spriteArea = 0;
	for (SpriteImp *sprite : atlas->sprites) {
		spriteArea += sprite->width * sprite->height;
	}
	uint32_t atlasArea = atlas->width * atlas->height;

	float wastedArea = 1.0f - (float)spriteArea / (float)atlasArea;

	if (atlas->width == MaxAtlasExtent && atlas->height == MaxAtlasExtent && wastedArea < 0.5f) {
		return 0.0f;
	}

	float score = brokenBatchesPerFrame + wastedArea * 10.0f;

	// Old atlases with only one texture
	if (list.frameIndex - atlas->frameCreated > 240 && atlas->sprites.size == 1) {
		score += 20.0f;
	}

	return score;
}

struct AtlasWithScore
{
	AtlasImp *atlas;
	float score;
};

static bool packSprite(RectPacker &packer, Sprite *sprite, uint32_t &x, uint32_t &y)
{
	uint32_t pad = AtlasPadding;
	uint32_t width = sprite->width + pad;
	uint32_t height = sprite->height + pad;
	width += (uint32_t)-(int32_t)width & (pad - 1);
	height += (uint32_t)-(int32_t)height & (pad - 1);
	return packer.pack(width, height, x, y);
}

static bool tryPackSprites(RectPacker &packer, sf::Slice<SpriteImp*> sprites)
{
	for (SpriteImp *sprite : sprites) {
		uint32_t x, y;
		if (!packSprite(packer, sprite, x, y)) {
			return false;
		}
	}
	return true;
}

static void sortSpritesToPack(sf::Slice<SpriteImp*> sprites)
{
	qsort(sprites.data, sprites.size, sizeof(SpriteImp*),
	[](const void *va, const void *vb) -> int {
		SpriteImp *a = (SpriteImp*)va, *b = (SpriteImp*)vb;
		uint32_t as = sf::max(a->width, a->height) * 32 + sf::min(a->width, a->height);
		uint32_t bs = sf::max(b->width, b->height) * 32 + sf::min(b->width, b->height);
		if (as < bs) return +1;
		if (as > bs) return -1;
		return 0;
	});
}

static void reassignAtlases(SpriteList &list)
{
	// Collect and score atlases
	sf::SmallArray<AtlasWithScore, 64> rating;
	rating.reserve(list.atlases.size);
	for (AtlasImp *atlas : list.atlases) {
		rating.push({ atlas, scoreAtlasForReassigning(list, atlas) });
	}

	// Reset batch break counters
	for (AtlasImp *atlas : list.atlases) {
		atlas->numBrokenBatches = 0;
	}

	// No point in reassigning one or less atlas
	if (list.atlases.size <= 1) return;

	// Sort by descending score
	qsort(rating.data, rating.size, sizeof(AtlasWithScore),
	[](const void *va, const void *vb) {
		AtlasWithScore *a = (AtlasWithScore*)va, *b = (AtlasWithScore*)vb;
		if (a->score < b->score) return +1; 
		if (a->score > b->score) return -1; 
		return 0;
	});

	// Nothing worth reassigning
	if (rating[0].score <= 5.0f) return;

	sf::Array<SpriteImp*> spritesToPack;

	// Insert sprites from atlases
	RectPacker packer;
	uint32_t numFit = 0;
	for (AtlasWithScore &atlasScore : rating) {
		spritesToPack.push(atlasScore.atlas->sprites);
		sortSpritesToPack(spritesToPack);

		AtlasImp *atlas = atlasScore.atlas;
		packer.reset(MaxAtlasExtent, MaxAtlasExtent);
		if (!tryPackSprites(packer, atlas->sprites)) break;
		numFit++;
	}

	spritesToPack.clear();
	for (uint32_t i = 0; i < numFit; i++) {
		spritesToPack.push(rating[i].atlas->sprites);
	}
	sortSpritesToPack(spritesToPack);

	// No sense to reassign only one (or zero) atlases
	if (numFit <= 1) return;

	// Try to find the minimum extent that works
	uint32_t minWidth = MaxAtlasExtent, minHeight = MaxAtlasExtent;
	while (minWidth / 2 >= MinAtlasExtent && minHeight / 2 >= MinAtlasExtent) {
		packer.reset(minWidth / 2, minHeight / 2);
		if (!tryPackSprites(packer, spritesToPack)) break;
		minWidth /= 2;
		minHeight /= 2;
	}

	if (minWidth / 2 >= MinAtlasExtent) {
		packer.reset(minWidth / 2, minHeight);
		if (tryPackSprites(packer, spritesToPack)) {
			minWidth /= 2;
		}
	}

	if (minHeight / 2 >= MinAtlasExtent) {
		packer.reset(minWidth, minHeight / 2);
		if (tryPackSprites(packer, spritesToPack)) {
			minHeight /= 2;
		}
	}

	// Pack the sprites for real
	packer.reset(minWidth, minHeight);
	for (SpriteImp *sprite : spritesToPack) {
		uint32_t x, y;
		bool res = packSprite(packer, sprite, x, y);
		sf_assert(res);
		sprite->prevX = sprite->x;
		sprite->prevY = sprite->y;
		sprite->x = x;
		sprite->y = y;
	}

	// Allocate the atlas
	AtlasImp *resultAtlas = new AtlasImp();
	resultAtlas->width = minWidth;
	resultAtlas->height = minHeight;
	resultAtlas->frameCreated = list.frameIndex;

	MipImage zeroImage(minWidth, minHeight);
	for (Image &image : zeroImage.levels) {
		image.clear();
	}

	// Initialize the GPU texture
	{
		sg_image_desc desc = { };
		desc.type = SG_IMAGETYPE_2D;
		desc.width = minWidth;
		desc.height = minHeight;
		desc.num_mipmaps = zeroImage.levels.size;
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
		desc.bqq_copy_target = true;
		for (uint32_t i = 0; i < zeroImage.levels.size; i++) {
			desc.content.subimage[0][i].ptr = zeroImage.levels[i].data;
			desc.content.subimage[0][i].size = zeroImage.levels[i].byteSize();
		}
		resultAtlas->image = sg_make_image(&desc);
	}

	sf::SmallArray<sg_bqq_subimage_rect, 32> rects;

	// Reassign and copy sprite data
	packer.reset(minWidth, minHeight);
	for (uint32_t i = 0; i < numFit; i++) {
		AtlasImp *atlas = rating[i].atlas;

		rects.clear();

		sg_image oldImg = atlas->image;
		for (SpriteImp *sprite : atlas->sprites) {
			sg_bqq_subimage_rect &rect = rects.pushUninit();
			rect.src_x = sprite->prevX;
			rect.src_y = sprite->prevY;
			rect.dst_x = sprite->x;
			rect.dst_y = sprite->y;
			rect.width = sprite->width;
			rect.height = sprite->height;

			sprite->atlas = resultAtlas;
			resultAtlas->sprites.push(sprite);
		}

		sg_bqq_subimage_copy_desc desc = { };
		desc.dst_image = resultAtlas->image;
		desc.src_image = atlas->image;

		// WebGL 1 can't copy from anything but the top-level...
		#if defined(SOKOL_GLES2)
			desc.num_mips = 1;
		#else
			desc.num_mips = AtlasLevels;
		#endif

		desc.rects = rects.data;
		desc.num_rects = rects.size;
		sg_bqq_copy_subimages(&desc);

		// Remove the old atlas from the list
		atlas->sprites.clear();
		sf::findRemoveSwap(list.atlases, atlas);
		list.atlasesToDelete.push({ atlas, list.frameIndex });
	}

	// Generate mipmaps afterwards on WebGL 1
	#if defined(SOKOL_GLES2)
		sg_bqq_generate_mipmaps(resultAtlas->image);
	#endif

	list.atlases.push(resultAtlas);
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

	// TODO: Only run this on some frames
	if (list.frameIndex % FramesBetweenReassign == 0 || true) {
		reassignAtlases(list);
	}
}

}