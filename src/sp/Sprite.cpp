#include "Sprite.h"

#include "ContentFile.h"

// TEMP TMEP
#include "ext/stb/stb_image.h"
#include "ext/sokol/sokol_gfx.h"

namespace sp {

struct SpriteImp : Sprite
{
	virtual void startLoadingImp() final;
	virtual void unloadImp() final;

	sg_image image;
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

	sg_image_desc desc = { };
	desc.width = width;
	desc.height = height;
	desc.min_filter = SG_FILTER_LINEAR;
	desc.mag_filter = SG_FILTER_LINEAR;
	desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
	desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
	desc.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
	desc.pixel_format = SG_PIXELFORMAT_RGBA8;
	desc.content.subimage[0][0].ptr = pixels;
	desc.content.subimage[0][0].size = width * height * 4;
	sg_image image = sg_make_image(&desc);

	stbi_image_free(pixels);

	imp->image = image;
	imp->finishLoadingImp();
}

void SpriteImp::startLoadingImp()
{
	ContentFile::load(name, &loadImp, this);
}

void SpriteImp::unloadImp()
{
	sg_destroy_image(image);
}

}
