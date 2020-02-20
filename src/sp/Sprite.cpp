#include "Sprite.h"

#include "ContentFile.h"
#include "Image.h"
#include "RectPacker.h"

#include "sf/Mutex.h"
#include "sf/Vector.h"
#include "sf/Sort.h"

#include "ext/stb/stb_image.h"
#include "ext/stb/stb_image_resize.h"
#include "ext/sokol/sokol_config.h"
#include "ext/sokol/sokol_gfx.h"
#include "sf/ext/mx/mx_platform.h"

namespace sp {

// -- Misc utility

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

struct CropRect
{
	uint32_t minX, minY;
	uint32_t maxX, maxY;
};

static CropRect cropAlpha(const uint8_t *data, uint32_t width, uint32_t height)
{
	CropRect rect;
	const uint8_t *alpha = data + 3;
	int32_t x, y;
	int32_t w = (int32_t)width;
	int32_t h = (int32_t)height;
	uint32_t stride = 4 * width;

	for (y = 0; y < h; y++) {
		const uint8_t *a = alpha + y * w * 4;
		for (x = 0; x < w; x++) {
			if (*a != 0) {
				rect.minY = (uint32_t)sf::max(y - 1, 0);
				y = h;
				break;
			}
			a += 4;
		}
	}

	if (y == h) {
		// Empty rectangle
		rect.minY = 0;
		rect.minX = 0;
		rect.maxX = 1;
		rect.maxY = 1;
		return rect;
	}

	for (y = h - 1; y >= 0; y--) {
		const uint8_t *a = alpha + y * w * 4;
		for (x = 0; x < w; x++) {
			if (*a != 0) {
				rect.maxY = (uint32_t)sf::min(y + 2, h);
				y = 0;
				break;
			}
			a += 4;
		}
	}

	for (x = 0; x < w; x++) {
		const uint8_t *a = alpha + x * 4;
		for (y = 0; y < h; y++) {
			if (*a != 0) {
				rect.minX = (uint32_t)sf::max(x - 1, 0);
				x = w;
				break;
			}
			a += stride;
		}
	}

	for (x = w - 1; x >= 0; x--) {
		const uint8_t *a = alpha + x * 4;
		for (y = 0; y < h; y++) {
			if (*a != 0) {
				rect.maxX = (uint32_t)sf::min(x + 2, w);
				x = 0;
				break;
			}
			a += stride;
		}
	}

	return rect;
}

static const uint32_t AtlasDeleteQueueFrames = 4;
static const uint32_t MinAtlasExtent = 64;
static const uint32_t AtlasLevels = 3;
static const uint32_t AtlasPadding = 1 << AtlasLevels;
static const uint32_t FramesBetweenReassign = 60;
static const uint32_t MinAtlasesToForceReassign = 16;
static const uint32_t ClearImageExtent = 256;

struct SpriteImp;
struct AtlasImp;

struct AtlasToDelete
{
	AtlasImp *atlas;
	uint32_t frameIndex;
};

struct SpriteContext
{
	sf::Mutex mutex;
	uint32_t frameIndex = 0;

	// Bookkeeping
	sf::Array<AtlasToDelete> atlasesToDelete;
	sf::Array<AtlasImp*> atlases;

	// Texture limits
	uint32_t maxSpriteExtent;
	uint32_t maxAtlasExtent;

	// For clearing atlases
	sg_image clearImage;
};

struct AtlasImp : Atlas
{
	sf::Array<SpriteImp*> sprites;

	uint32_t numBrokenBatches = 0;
	uint32_t frameCreated = 0;
};

struct SpriteImp : Sprite
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;

	// Temporary, used during reassignment
	uint32_t prevX = 0, prevY = 0;
};

SpriteContext g_spriteContext;

uint32_t SpriteProps::hash() const
{
	uint32_t h = 0;
	h = sf::hashCombine(h, sf::hash(atlasName));
	return h;
}

bool SpriteProps::equal(const AssetProps &rhs) const
{
	const SpriteProps &r = (const SpriteProps&)rhs;
	if (atlasName != r.atlasName) return false;
	return true;
}

void SpriteProps::copyTo(AssetProps *uninitDst) const
{
	SpriteProps *dst = (SpriteProps*)uninitDst;
	new (dst) SpriteProps();
	dst->atlasName.append(atlasName);
}


AssetType Sprite::AssetType = { "Sprite", sizeof(SpriteImp), sizeof(Sprite::PropType),
	[](Asset *a) { new ((SpriteImp*)a) SpriteImp(); }
};

static void loadImp(void *user, const ContentFile &file)
{
	SpriteContext &ctx = g_spriteContext;

	SpriteImp *imp = (SpriteImp*)user;
	if (!file.isValid()) {
		imp->assetFailLoading();
		return;
	}

	int width, height;
	stbi_uc *pixels = stbi_load_from_memory((const stbi_uc*)file.data, (int)file.size, &width, &height, NULL, 4);
	if (!pixels) return;

	if (width > (int)ctx.maxSpriteExtent || height > (int)ctx.maxSpriteExtent) {
		uint32_t srcWidth = width, srcHeight = height;
		double aspect = (double)width / (double)height;
		if (width >= height) {
			width = ctx.maxSpriteExtent;
			height = (uint32_t)(ctx.maxSpriteExtent / aspect);
		} else {
			height = ctx.maxSpriteExtent;
			width = (uint32_t)(ctx.maxSpriteExtent * aspect);
		}

		stbi_uc *newPixels = (stbi_uc*)malloc(width * height * 4);
		stbir_resize_uint8_srgb(
			pixels, srcWidth, srcHeight, 0,
			newPixels, width, height, 0,
			4, 3, 0);

		free(pixels);
		pixels = newPixels;
	}

	CropRect rect = cropAlpha(pixels, width, height);
	uint32_t stride = width * 4;

	imp->minVert.x = (float)(rect.minX - 0.5f) / (float)width;
	imp->minVert.y = (float)(rect.minY - 0.5f) / (float)height;
	imp->maxVert.x = (float)(rect.maxX + 0.5f) / (float)width;
	imp->maxVert.y = (float)(rect.maxY + 0.5f) / (float)height;

	width = rect.maxX - rect.minX;
	height = rect.maxY - rect.minY;

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

	stbi_uc *basePixels = pixels + rect.minY * stride + rect.minX * 4;
	image.levels[0].blit(0, 0, width, height, basePixels, stride);
	image.levels[0].premultiply();
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
		desc.wrap_u = SG_WRAP_REPEAT;
		desc.wrap_v = SG_WRAP_REPEAT;
		desc.wrap_w = SG_WRAP_REPEAT;
		desc.label = "Single Atlas";
		for (uint32_t i = 0; i < image.levels.size; i++) {
			desc.content.subimage[0][i].ptr = image.levels[i].data;
			desc.content.subimage[0][i].size = (int)image.levels[i].byteSize();
		}
		atlas->image = sg_make_image(&desc);
	}

	free(pixels);

	atlas->sprites.push(imp);
	imp->atlas = atlas;

	{
		sf::MutexGuard mg(ctx.mutex);
		ctx.atlases.push(atlas);
		atlas->frameCreated = ctx.frameIndex;
	}

	imp->assetFinishLoading();
}

void SpriteImp::assetStartLoading()
{
	ContentFile::load(name, &loadImp, this);
}

void SpriteImp::assetUnload()
{
	SpriteContext &ctx = g_spriteContext;
	sf::MutexGuard mg(ctx.mutex);

	// Remove from the atlas
	AtlasImp *atlasImp = (AtlasImp*)atlas;
	sf::findRemoveSwap(atlasImp->sprites, this);

	// If this was the last sprite in the atlas delete it
	if (atlasImp->sprites.size == 0) {
		sf::findRemoveSwap(ctx.atlases, atlasImp);
		ctx.atlasesToDelete.push({ atlasImp, ctx.frameIndex });
	}
}

void Atlas::brokeBatch()
{
	AtlasImp *imp = (AtlasImp*)this;
	mxa_inc32_nf(&imp->numBrokenBatches);
}

void Atlas::getAtlases(sf::Array<Atlas*> &atlases)
{
	SpriteContext &ctx = g_spriteContext;
	sf::MutexGuard mg(ctx.mutex);

	for (AtlasImp *atlas : ctx.atlases) {
		atlases.push(atlas);
	}
}

void Sprite::globalInit()
{
	sg_limits limits = sg_query_limits();

	SpriteContext &ctx = g_spriteContext;
	ctx.maxSpriteExtent = sf::min(256u, limits.max_image_size_2d);
	ctx.maxAtlasExtent = sf::min(2048u, limits.max_image_size_2d);

	{
		MipImage clearImage(ClearImageExtent, ClearImageExtent);
		sg_image_desc desc = { };
		desc.type = SG_IMAGETYPE_2D;
		desc.width = ClearImageExtent;
		desc.height = ClearImageExtent;
		desc.num_mipmaps = clearImage.levels.size;
		desc.usage = SG_USAGE_IMMUTABLE;
		desc.pixel_format = SG_PIXELFORMAT_RGBA8;
		desc.label = "Clear texture";
		for (uint32_t i = 0; i < clearImage.levels.size; i++) {
			desc.content.subimage[0][i].ptr = clearImage.levels[i].data;
			desc.content.subimage[0][i].size = (int)clearImage.levels[i].byteSize();
		}
		ctx.clearImage = sg_make_image(&desc);
	}
}

void Sprite::globalCleanup()
{
	SpriteContext &ctx = g_spriteContext;

	sg_destroy_image(ctx.clearImage);
}

static float scoreAtlasForReassigning(SpriteContext &ctx, AtlasImp *atlas)
{
	uint32_t brokenBatches = mxa_load32_nf(&atlas->numBrokenBatches);
	float brokenBatchesPerFrame = (float)brokenBatches / (float)FramesBetweenReassign;

	uint32_t spriteArea = 0;
	for (SpriteImp *sprite : atlas->sprites) {
		spriteArea += sprite->width * sprite->height;
	}
	uint32_t atlasArea = atlas->width * atlas->height;

	float wastedArea = 1.0f - (float)spriteArea / (float)atlasArea;

	if (atlas->width == ctx.maxAtlasExtent && atlas->height == ctx.maxAtlasExtent && wastedArea < 0.5f) {
		return 0.0f;
	}

	float score = brokenBatchesPerFrame + wastedArea * 10.0f;

	// Old atlases with only one texture
	if (ctx.frameIndex - atlas->frameCreated > 240 && atlas->sprites.size == 1) {
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
	sf::sortByRev(sprites, [](const SpriteImp *s) {
		return sf::min(s->width, s->height) * 64 + sf::max(s->width, s->height);
	});
}

static void reassignAtlases(SpriteContext &ctx)
{
	// Collect and score atlases
	sf::SmallArray<AtlasWithScore, 64> rating;
	rating.reserve(ctx.atlases.size);
	for (AtlasImp *atlas : ctx.atlases) {
		rating.push({ atlas, scoreAtlasForReassigning(ctx, atlas) });
	}

	// Reset batch break counters
	for (AtlasImp *atlas : ctx.atlases) {
		atlas->numBrokenBatches = 0;
	}

	// No point in reassigning one or less atlas
	if (ctx.atlases.size <= 1) return;

	// Sort by descending score
	sf::sortByRev(rating, [](const AtlasWithScore &a) {
		return a.score;
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
		packer.reset(ctx.maxAtlasExtent, ctx.maxAtlasExtent);
		if (!tryPackSprites(packer, spritesToPack)) break;
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
	uint32_t minWidth = ctx.maxAtlasExtent, minHeight = ctx.maxAtlasExtent;
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
	resultAtlas->frameCreated = ctx.frameIndex;

	MipImage zeroImage(minWidth, minHeight);
	for (Image &image : zeroImage.levels) {
		image.clear();
	}
	uint32_t numMips = 1;
	{
		uint32_t w = minWidth, h = minHeight;
		while (w > 1 || h > 1) {
			numMips++;
			w = sf::max(w / 2, 1u);
			h = sf::max(h / 2, 1u);
		}
	}

	// Initialize the GPU texture
	{
		sg_image_desc desc = { };
		desc.type = SG_IMAGETYPE_2D;
		desc.width = minWidth;
		desc.height = minHeight;
		desc.num_mipmaps = numMips;
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
		resultAtlas->image = sg_make_image(&desc);
	}

	sf::SmallArray<sg_bqq_subimage_rect, 32> rects;

	// Clear the atlas first
	{
		for (uint32_t y = 0; y < minHeight; y += ClearImageExtent) {
			for (uint32_t x = 0; x < minWidth; x += ClearImageExtent) {
				sg_bqq_subimage_rect &rect = rects.push();
				rect.src_x = 0;
				rect.src_y = 0;
				rect.dst_x = x;
				rect.dst_y = y;
				rect.width = sf::min(minWidth - x, ClearImageExtent);
				rect.height = sf::min(minHeight - y, ClearImageExtent);
			}
		}

		sg_bqq_subimage_copy_desc desc = { };
		desc.dst_image = resultAtlas->image;
		desc.src_image = ctx.clearImage;
		// WebGL 1 can't copy from anything but the top-level...
		#if defined(SOKOL_GLES2)
			desc.num_mips = 1;
		#else
			desc.num_mips = AtlasLevels;
		#endif

		desc.rects = rects.data;
		desc.num_rects = rects.size;
		sg_bqq_copy_subimages(&desc);
	}

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

		// Remove the old atlas from the ctx
		atlas->sprites.clear();
		sf::findRemoveSwap(ctx.atlases, atlas);
		ctx.atlasesToDelete.push({ atlas, ctx.frameIndex });
	}

	// Generate mipmaps afterwards on WebGL 1
	#if defined(SOKOL_GLES2)
		sg_bqq_generate_mipmaps(resultAtlas->image);
	#endif

	ctx.atlases.push(resultAtlas);
}

void Sprite::globalUpdate()
{
	SpriteContext &ctx = g_spriteContext;
	sf::MutexGuard mg(ctx.mutex);

	ctx.frameIndex++;
}

}

