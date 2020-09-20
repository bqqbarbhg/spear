#include "GIMaterial.h"

#include "sp/ContentFile.h"
#include "sp/Renderer.h"
#include "sf/Mutex.h"

#include "ext/sokol/sokol_gfx.h"
#include "ext/sp_tools_common.h"

namespace cl {

struct GIMaterialAtlasTexture
{
	sp::Texture texture;
	uint32_t textureExtent;
	uint32_t resolutionX, resolutionY;
	sg_pixel_format pixelFormat = SG_PIXELFORMAT_NONE;

	void init(uint32_t numSlotsX, uint32_t numSlotsY, uint32_t extent, sg_pixel_format format, const char *label)
	{
		pixelFormat = format;
		textureExtent = extent;
		resolutionX = numSlotsX * extent;
		resolutionY = numSlotsY * extent;

		sg_image_desc desc = { };
		desc.label = label;
		desc.pixel_format = format;
		desc.width = (int)resolutionX;
		desc.height = (int)resolutionY;
		desc.num_mipmaps = 1;
		desc.bqq_copy_target = true;
		desc.mag_filter = SG_FILTER_LINEAR;
		desc.min_filter = SG_FILTER_LINEAR;
		desc.max_lod = 0.0f;
		texture.init(desc);
	}
};

struct GIMaterialContext
{
	sf::Mutex mutex;

	uint32_t numSlotsX = 16, numSlotsY = 16;

	GIMaterialAtlasTexture atlas;

	sf::Array<uint32_t> freeSlots;
	uint32_t nextFreeSlot = 0;

	uint32_t allocSlot() {
		if (freeSlots.size > 0) {
			return freeSlots.popValue();
		} else {
			uint32_t res = nextFreeSlot++;
			sf_assert(res < numSlotsX * numSlotsY);
			return res;
		}
	}

	void freeSlot(uint32_t slot) {
		freeSlots.push(slot);
	}
};

GIMaterialContext g_giMaterialContext;

struct GIMaterialImp : GIMaterial
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;

	uint32_t slot = ~0u;
};

sp::AssetType GIMaterial::SelfType = { "GIMaterial", sizeof(GIMaterialImp), sizeof(GIMaterial::PropType),
	[](Asset *a) { new ((GIMaterialImp*)a) GIMaterialImp(); }
};

static void loadTextureImp(void *user, const sp::ContentFile &file)
{
	GIMaterialImp *imp = (GIMaterialImp*)user;

	if (file.size > 0) {
		GIMaterialContext &ctx = g_giMaterialContext;
		GIMaterialAtlasTexture &atlas = ctx.atlas;

		sptex_util su;
		sptex_util_init(&su, file.data, file.size);

		sg_image_desc d = { };
		d.pixel_format = atlas.pixelFormat;
		d.num_mipmaps = 1;
		d.width = atlas.textureExtent;
		d.height = atlas.textureExtent;

		sptex_header header = sptex_decode_header(&su);
		uint32_t extent = header.info.width;
		sf_assert(extent == header.info.height);
		sf_assert(extent >= atlas.textureExtent);

		uint32_t mipDrop = 0;
		while (extent > atlas.textureExtent) {
			extent /= 2;
			mipDrop++;
		}
		sf_assert(extent == atlas.textureExtent);

		{
			d.content.subimage[0][0].ptr = sptex_decode_mip(&su, mipDrop);
			d.content.subimage[0][0].size = header.s_mips[mipDrop].uncompressed_size;
		}

		uint32_t offsetX = (imp->slot % ctx.numSlotsX) * atlas.textureExtent;
		uint32_t offsetY = (imp->slot / ctx.numSlotsY) * atlas.textureExtent;

		if (!spfile_util_failed(&su.file)) {
			sp::ContentFile::mainThreadCallbackFunc([&](){
				sg_bqq_update_subimage(atlas.texture.image, &d, (int)offsetX, (int)offsetY);
			});
		}

		spfile_util_free(&su.file);
	}

	imp->assetFinishLoading();
}

static void loadAlbedoImp(void *user, const sp::ContentFile &file) { loadTextureImp(user, file); }

void GIMaterialImp::assetStartLoading()
{
    GIMaterialContext &ctx = g_giMaterialContext;
	sf::SmallStringBuf<256> path;

	{
		sf::MutexGuard mg(ctx.mutex);
		slot = ctx.allocSlot();
	}

    sf::Vec2 rcpSize = sf::Vec2(1.0f) / sf::Vec2((float)ctx.numSlotsX, (float)ctx.numSlotsY);
    sf::Vec2i offset = { (int32_t)(slot % ctx.numSlotsX), (int32_t)(slot / ctx.numSlotsX) };
    uvBase = sf::Vec2(offset) * rcpSize;
    uvScale = rcpSize;

    path.clear(); path.format("%s.%s.sptex", name.data, getPixelFormatSuffix(ctx.atlas.pixelFormat));
	sp::ContentFile::loadAsync(path, &loadAlbedoImp, this);
}

void GIMaterialImp::assetUnload()
{
	GIMaterialContext &ctx = g_giMaterialContext;
	sf::MutexGuard mg(ctx.mutex);

	if (slot != ~0u) {
		ctx.freeSlot(slot);
		slot = ~0u;
	}
}

sg_image GIMaterial::getAtlasImage()
{
	GIMaterialContext &ctx = g_giMaterialContext;
	return ctx.atlas.texture.image;
}

uint32_t GIMaterial::getAtlasPixelFormat()
{
	GIMaterialContext &ctx = g_giMaterialContext;
	return (uint32_t)ctx.atlas.pixelFormat;
}

void GIMaterial::globalInit()
{
	GIMaterialContext &ctx = g_giMaterialContext;

    sg_pixel_format giFormats[] = {
        // SG_PIXELFORMAT_BQQ_BC1_SRGB,
        // SG_PIXELFORMAT_BQQ_ASTC_4X4_SRGB,
        SG_PIXELFORMAT_BQQ_SRGBA8,
    };

	sg_pixel_format atlasFormat = SG_PIXELFORMAT_NONE;
    for (sg_pixel_format format : giFormats) {
        if (!sg_query_pixelformat(format).sample) continue;
		atlasFormat = format;
        break;
    }

	uint32_t resolution = 32;

	ctx.atlas.init(ctx.numSlotsX, ctx.numSlotsY, resolution, atlasFormat, "GIMaterial albedo");
}

void GIMaterial::globalCleanup()
{
	g_giMaterialContext.~GIMaterialContext();
	new (&g_giMaterialContext) GIMaterialContext();
}

}
