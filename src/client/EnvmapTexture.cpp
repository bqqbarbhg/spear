#include "EnvmapTexture.h"

#include "ext/sp_tools_common.h"

#include "sp/ContentFile.h"

namespace cl {

sg_pixel_format EnvmapTexture::pixelFormat;
sg_image EnvmapTexture::defaultImage;

struct EnvmapTextureImp : EnvmapTexture
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;
};

sp::AssetType EnvmapTexture::SelfType = { "EnvmapTexture", sizeof(EnvmapTextureImp), sizeof(EnvmapTexture::PropType),
	[](Asset *a) { new ((EnvmapTextureImp*)a) EnvmapTextureImp(); }
};

static void loadTextureImp(void *user, const sp::ContentFile &file)
{
	EnvmapTextureImp *imp = (EnvmapTextureImp*)user;
    bool ok = false;

	if (file.size > 0) {
		sptex_util su;
		sptex_util_init(&su, file.data, file.size);

		sptex_header header = sptex_decode_header(&su);

		sg_image_desc d = { };
		d.pixel_format = EnvmapTexture::pixelFormat;
		d.num_mipmaps = header.info.num_mips;
		d.width = header.info.width;
		d.height = header.info.height;
		d.mag_filter = SG_FILTER_LINEAR;
		d.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
		d.type = SG_IMAGETYPE_CUBE;
		d.label = imp->name.data;

		for (uint32_t mipI = 0; mipI < (uint32_t)d.num_mipmaps; mipI++) {
			char *data = sptex_decode_mip(&su, mipI);
			size_t mipSize = header.s_mips[mipI].uncompressed_size / 6;
			if (data) {
				for (uint32_t faceI = 0; faceI < 6; faceI++) {
					d.content.subimage[faceI][mipI].ptr = data;
					d.content.subimage[faceI][mipI].size = (int)mipSize;
					data += mipSize;
				}
			}
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

void EnvmapTextureImp::assetStartLoading()
{
	sf::SmallStringBuf<256> path;

	if (name.size() > 0) {
		path.clear(); path.format("%s.%s.sptex", name.data, sp::getPixelFormatSuffix(EnvmapTexture::pixelFormat));
		sp::ContentFile::loadAsync(path, &loadTextureImp, this);
	} else {
		assetFailLoading();
	}
}

void EnvmapTextureImp::assetUnload()
{
	if (image.id != 0) {
		sg_destroy_image(image);
		image.id = 0;
	}
}

void EnvmapTexture::globalInit()
{
    sg_pixel_format formats[] = {
        SG_PIXELFORMAT_RG11B10F,
    };

    for (sg_pixel_format format : formats) {
        if (!sg_query_pixelformat(format).sample) continue;
        EnvmapTexture::pixelFormat = format;
        break;
    }

    {
        sg_image_desc desc = { };
		desc.width = 1;
		desc.height = 1;
		desc.num_mipmaps = 1;
		desc.mag_filter = SG_FILTER_LINEAR;
		desc.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
		desc.type = SG_IMAGETYPE_CUBE;
		desc.max_anisotropy = 4;

		{
			const unsigned char content[] = { 0x00, 0x00, 0x00, 0x00 };
			desc.pixel_format = SG_PIXELFORMAT_BQQ_SRGBA8;
			for (uint32_t i = 0; i < 6; i++) {
				desc.content.subimage[i][0].ptr = content;
				desc.content.subimage[i][0].size = sizeof(content);
			}
			EnvmapTexture::defaultImage = sg_make_image(&desc);
		}

    }
}

void EnvmapTexture::globalCleanup()
{
	sg_destroy_image(EnvmapTexture::defaultImage);
}

}
