#include "MiscTexture.h"

#include "ext/sp_tools_common.h"

#include "sp/ContentFile.h"

namespace cl {

struct MiscTextureImp : MiscTexture
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;
};

sp::AssetType MiscTexture::SelfType = { "MiscTexture", sizeof(MiscTextureImp), sizeof(MiscTexture::PropType),
	[](Asset *a) { new ((MiscTextureImp*)a) MiscTextureImp(); }
};

uint32_t MiscTextureProps::hash() const
{
	uint32_t h = 0;
	h = sf::hashCombine(h, sf::hash((uint32_t)minFilter));
	h = sf::hashCombine(h, sf::hash((uint32_t)magFilter));
	return h;
}

bool MiscTextureProps::equal(const AssetProps &rhs) const
{
	const MiscTextureProps &r = (const MiscTextureProps&)rhs;
	if (minFilter != r.minFilter) return false;
	if (magFilter != r.magFilter) return false;
	return true;
}

void MiscTextureProps::copyTo(AssetProps *uninitDst) const
{
	MiscTextureProps *dst = (MiscTextureProps*)uninitDst;
	new (dst) MiscTextureProps(*this);
}

static void loadTextureImp(void *user, const sp::ContentFile &file)
{
	MiscTextureImp *imp = (MiscTextureImp*)user;
    bool ok = false;
	MiscTextureProps &props = *(MiscTextureProps*)imp->props;

	if (file.size > 0) {
		sptex_util su;
		sptex_util_init(&su, file.data, file.size);

		sptex_header header = sptex_decode_header(&su);

		sg_pixel_format pixelFormat = SG_PIXELFORMAT_NONE;
		if (header.info.format == SP_FORMAT_RGBA8_UNORM) {
			pixelFormat = SG_PIXELFORMAT_RGBA8;
		}

		if (pixelFormat != SG_PIXELFORMAT_NONE) {
			imp->pixelFormat = pixelFormat;

			sg_image_desc d = { };
			d.pixel_format = pixelFormat;
			d.num_mipmaps = header.info.num_mips;
			d.width = header.info.width;
			d.height = header.info.height;
			d.mag_filter = props.magFilter;
			d.min_filter = props.minFilter;
			d.label = imp->name.data;

			uint32_t mipDrop = 0;

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
		}

		spfile_util_free(&su.file);
	}

    if (ok) {
    	imp->assetFinishLoading();
    } else {
        imp->assetFailLoading();
    }
}

void MiscTextureImp::assetUnload()
{
	if (image.id != 0) {
		sg_destroy_image(image);
		image.id = 0;
	}
	pixelFormat = SG_PIXELFORMAT_NONE;
}

void MiscTextureImp::assetStartLoading()
{
	sf::SmallStringBuf<256> path;

	if (name.size() > 0) {
		path.clear(); path.format("%s.sptex", name.data);
		sp::ContentFile::loadAsync(path, &loadTextureImp, this);
	} else {
		assetFailLoading();
	}
}

}
