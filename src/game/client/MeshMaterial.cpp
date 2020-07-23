#include "MeshMaterial.h"

#include "ext/sokol/sokol_gfx.h"
#include "ext/sp_tools_common.h"
#include "sp/ContentFile.h"
#include "sf/String.h"

#if SF_OS_EMSCRIPTEN
	#include <emscripten/emscripten.h>
#endif

extern bool g_hack_hd;

namespace cl {

const char *getPixelFormatSuffix(sg_pixel_format format)
{
    switch (format) {
    case SG_PIXELFORMAT_RGBA8: return "rgba8";
    case SG_PIXELFORMAT_BQQ_SRGBA8: return "rgba8";
    case SG_PIXELFORMAT_BC1_RGBA: return "bc1";
    case SG_PIXELFORMAT_BQQ_BC1_SRGB: return "bc1";
    case SG_PIXELFORMAT_BC3_RGBA: return "bc3";
    case SG_PIXELFORMAT_BQQ_BC3_SRGB: return "bc3";
    case SG_PIXELFORMAT_BC5_RG: return "bc5";
    case SG_PIXELFORMAT_BC5_RGSN: return "bc5";
    case SG_PIXELFORMAT_BQQ_BC7_SRGB: return "bc7";
    case SG_PIXELFORMAT_BQQ_ASTC_4X4_RGBA: return "astc4x4";
    case SG_PIXELFORMAT_BQQ_ASTC_4X4_SRGB: return "astc4x4";
    case SG_PIXELFORMAT_BQQ_ASTC_8X8_RGBA: return "astc8x8";
    case SG_PIXELFORMAT_BQQ_ASTC_8X8_SRGB: return "astc8x8";
    default:
        sf_failf("Invalid pixel format: %u", (uint32_t)format);
        return "";
    }
}

sg_pixel_format MeshMaterial::materialFormats[(uint32_t)MaterialTexture::Count];
sg_image MeshMaterial::defaultImages[(uint32_t)MaterialTexture::Count];

struct MeshMaterialImp : MeshMaterial
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;

	uint32_t textureLoadedMask = 0;
};

sp::AssetType MeshMaterial::SelfType = { "MeshMaterial", sizeof(MeshMaterialImp), sizeof(MeshMaterial::PropType),
	[](Asset *a) { new ((MeshMaterialImp*)a) MeshMaterialImp(); }
};

static void loadTextureImp(void *user, const sp::ContentFile &file, MaterialTexture texture)
{
	MeshMaterialImp *imp = (MeshMaterialImp*)user;

	uint32_t textureBit = 1 << (uint32_t)texture;
	sf_assert((imp->textureLoadedMask & textureBit) == 0);
	imp->textureLoadedMask |= textureBit;
	bool allLoaded = imp->textureLoadedMask == (1 << (uint32_t)MaterialTexture::Count) - 1;

	if (file.size > 0) {
		sptex_util su;
		sptex_util_init(&su, file.data, file.size);

		sptex_header header = sptex_decode_header(&su);

		sg_image_desc d = { };
		d.pixel_format = MeshMaterial::materialFormats[(uint32_t)texture];
		d.num_mipmaps = header.info.num_mips;
		d.width = header.info.width;
		d.height = header.info.height;
		d.mag_filter = SG_FILTER_LINEAR;
		d.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
		d.max_anisotropy = 4;

        sf::SmallStringBuf<128> name;
        name.append(imp->name);
        switch (texture) {
        case MaterialTexture::Albedo: name.append(" albedo"); break;
        case MaterialTexture::Normal: name.append(" normal"); break;
        case MaterialTexture::Mask: name.append(" mask"); break;
        case MaterialTexture::Count: /* nop */ break;
        }
        d.label = name.data;

		uint32_t mipDrop = 0;

        // TODO: Mip drop
#if 0
		while (extent > atlas.textureExtent) {
			extent /= 2;
			mipDrop++;
		}
		sf_assert(extent == atlas.textureExtent);
#endif

		for (uint32_t mipI = 0; mipI < (uint32_t)d.num_mipmaps; mipI++) {
			d.content.subimage[0][mipI].ptr = sptex_decode_mip(&su, mipDrop + mipI);
			d.content.subimage[0][mipI].size = header.s_mips[mipDrop + mipI].uncompressed_size;
		}

        imp->images[(uint32_t)texture] = sg_make_image(&d);

		spfile_util_free(&su.file);
	}

	if (allLoaded) {
		imp->assetFinishLoading();
	}
}

static void loadAlbedoImp(void *user, const sp::ContentFile &file) { loadTextureImp(user, file, MaterialTexture::Albedo); }
static void loadNormalImp(void *user, const sp::ContentFile &file) { loadTextureImp(user, file, MaterialTexture::Normal); }
static void loadMaskImp(void *user, const sp::ContentFile &file) { loadTextureImp(user, file, MaterialTexture::Mask); }

void MeshMaterialImp::assetStartLoading()
{
	sf::SmallStringBuf<256> path;

    uint32_t resolution = g_hack_hd ? 1024u : 512u;

    path.clear(); path.format("%s_albedo.%s.%u.sptex", name.data, getPixelFormatSuffix(MeshMaterial::materialFormats[(uint32_t)MaterialTexture::Albedo]), resolution);
	sp::ContentFile::loadMainThread(path, &loadAlbedoImp, this);

    path.clear(); path.format("%s_normal.%s.%u.sptex", name.data, getPixelFormatSuffix(MeshMaterial::materialFormats[(uint32_t)MaterialTexture::Normal]), resolution);
	sp::ContentFile::loadMainThread(path, &loadNormalImp, this);

    path.clear(); path.format("%s_mask.%s.%u.sptex", name.data, getPixelFormatSuffix(MeshMaterial::materialFormats[(uint32_t)MaterialTexture::Mask]), resolution);
	sp::ContentFile::loadMainThread(path, &loadMaskImp, this);
}

void MeshMaterialImp::assetUnload()
{
    for (uint32_t i = 0; i < (uint32_t)MaterialTexture::Count; i++) {
        if (images[i].id != 0) {
			sg_destroy_image(images[i]);
            images[i].id = 0;
        }
    }
}

#if SF_OS_EMSCRIPTEN
EM_JS(int, spear_emHasBC5, (void), {
    return navigator.userAgent.indexOf("Macintosh") < 0 ? 1 : 0;
});
#endif

void MeshMaterial::globalInit()
{
    sg_pixel_format albedoFormats[] = {
        SG_PIXELFORMAT_BQQ_BC7_SRGB,
        SG_PIXELFORMAT_BQQ_BC1_SRGB,
        SG_PIXELFORMAT_BQQ_ASTC_4X4_SRGB,
        SG_PIXELFORMAT_BQQ_SRGBA8,
    };
    
    sg_pixel_format normalFormats[] = {
        SG_PIXELFORMAT_BC5_RG,
        SG_PIXELFORMAT_BQQ_ASTC_4X4_RGBA,
        SG_PIXELFORMAT_BC3_RGBA,
        SG_PIXELFORMAT_RGBA8,
    };
    
    sg_pixel_format maskFormats[] = {
        SG_PIXELFORMAT_BC3_RGBA,
        SG_PIXELFORMAT_BQQ_ASTC_8X8_SRGB,
        SG_PIXELFORMAT_RGBA8,
    };

    for (sg_pixel_format format : albedoFormats) {
        if (!sg_query_pixelformat(format).sample) continue;
        MeshMaterial::materialFormats[(uint32_t)MaterialTexture::Albedo] = format;
        break;
    }

    for (sg_pixel_format format : normalFormats) {
        if (!sg_query_pixelformat(format).sample) continue;

		#if SF_OS_EMSCRIPTEN
            if (format == SG_PIXELFORMAT_BC5_RG && !spear_emHasBC5()) continue;
		#endif

        MeshMaterial::materialFormats[(uint32_t)MaterialTexture::Normal] = format;
        break;
    }
    
    for (sg_pixel_format format : maskFormats) {
        if (!sg_query_pixelformat(format).sample) continue;
        MeshMaterial::materialFormats[(uint32_t)MaterialTexture::Mask] = format;
        break;
    }

    {
        sg_image_desc desc = { };
		desc.width = 1;
		desc.height = 1;
		desc.num_mipmaps = 1;
		desc.mag_filter = SG_FILTER_LINEAR;
		desc.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
		desc.max_anisotropy = 4;

		{
			const unsigned char content[] = { 0x80, 0x80, 0x80, 0xff };
			desc.pixel_format = SG_PIXELFORMAT_BQQ_SRGBA8;
            desc.content.subimage[0][0].ptr = content;
            desc.content.subimage[0][0].size = sizeof(content);
			MeshMaterial::defaultImages[(uint32_t)MaterialTexture::Albedo] = sg_make_image(&desc);
		}

		{
			const unsigned char content[] = { 0x80, 0x80, 0x00, 0x00 };
			desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            desc.content.subimage[0][0].ptr = content;
            desc.content.subimage[0][0].size = sizeof(content);
			MeshMaterial::defaultImages[(uint32_t)MaterialTexture::Normal] = sg_make_image(&desc);
		}

		{
			const unsigned char content[] = { 0x00, 0xff, 0x00, 0x80 };
			desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            desc.content.subimage[0][0].ptr = content;
            desc.content.subimage[0][0].size = sizeof(content);
			MeshMaterial::defaultImages[(uint32_t)MaterialTexture::Mask] = sg_make_image(&desc);
		}

    }
}

void MeshMaterial::globalCleanup()
{
    for (uint32_t i = 0; i < (uint32_t)MaterialTexture::Count; i++) {
		sg_destroy_image(MeshMaterial::defaultImages[i]);
    }
}

}

