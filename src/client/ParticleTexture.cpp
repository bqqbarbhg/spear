#include "ParticleTexture.h"

#include "ext/sp_tools_common.h"

#include "client/ClientSettings.h"

#include "sp/ContentFile.h"

namespace cl {

sg_pixel_format ParticleTexture::pixelFormat;
sg_image ParticleTexture::defaultImage;

struct ParticleTextureImp : ParticleTexture
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;
};

sp::AssetType ParticleTexture::SelfType = { "ParticleTexture", sizeof(ParticleTextureImp), sizeof(ParticleTexture::PropType),
	[](Asset *a) { new ((ParticleTextureImp*)a) ParticleTextureImp(); }
};

static void loadTextureImp(void *user, const sp::ContentFile &file)
{
	ParticleTextureImp *imp = (ParticleTextureImp*)user;
    bool ok = false;

	if (file.size > 0) {
		sptex_util su;
		sptex_util_init(&su, file.data, file.size);

		sptex_header header = sptex_decode_header(&su);

		sg_image_desc d = { };
		d.pixel_format = ParticleTexture::pixelFormat;
		d.num_mipmaps = header.info.num_mips;
		d.width = header.info.width;
		d.height = header.info.height;
		d.label = imp->name.data;
        initSampler(d, g_settings);

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
        
        if (!spfile_util_failed(&su.file)) {

			sp::ContentFile::mainThreadCallbackFunc([&](){
				imp->image = sg_make_image(&d);
			});

            ok = true;
        }

		spfile_util_free(&su.file);
	}

    if (ok) {
    	imp->assetFinishLoading();
    } else {
        imp->assetFailLoading();
    }
}

void ParticleTextureImp::assetStartLoading()
{
	sf::SmallStringBuf<256> path;

	if (name.size() > 0) {
		path.clear(); path.format("%s.%s.sptex", name.data, sp::getPixelFormatSuffix(ParticleTexture::pixelFormat));
		sp::ContentFile::loadAsync(path, &loadTextureImp, this);
	} else {
		assetFailLoading();
	}
}

void ParticleTextureImp::assetUnload()
{
	if (image.id != 0) {
		sg_destroy_image(image);
		image.id = 0;
	}
}

void ParticleTexture::globalInit()
{
    sg_pixel_format formats[] = {
        SG_PIXELFORMAT_BQQ_BC3_SRGB,
        SG_PIXELFORMAT_BQQ_ASTC_4X4_SRGB,
        SG_PIXELFORMAT_BQQ_SRGBA8,
    };

    for (sg_pixel_format format : formats) {
        if (!sg_query_pixelformat(format).sample) continue;
        ParticleTexture::pixelFormat = format;
        break;
    }

    {
        sg_image_desc desc = { };
		desc.width = 1;
		desc.height = 1;
		desc.num_mipmaps = 1;
		desc.mag_filter = SG_FILTER_LINEAR;
		desc.min_filter = SG_FILTER_LINEAR;

		{
			const unsigned char content[] = { 0xff, 0xff, 0xff, 0xff };
			desc.pixel_format = SG_PIXELFORMAT_BQQ_SRGBA8;
            desc.content.subimage[0][0].ptr = content;
            desc.content.subimage[0][0].size = sizeof(content);
			ParticleTexture::defaultImage = sg_make_image(&desc);
		}

    }
}

void ParticleTexture::globalCleanup()
{
	sg_destroy_image(ParticleTexture::defaultImage);
}

}
