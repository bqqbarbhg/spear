#include "Sprite.h"

#include "ext/stb/stb_image.h"
#include "ext/sokol/sokol_gfx.h"

#include "sf/Mutex.h"
#include "sf/Array.h"
#include "sf/Vector.h"

struct SpriteImp
{
	SpriteInfo info;
	bool loaded = false;
};

struct SpriteData
{
	sf::Mutex mutex;
	sf::Array<SpriteImp> sprites;
};
static SpriteData g_data;

struct SpriteAssetType : AssetType
{
	virtual void load(Asset asset, const AssetLoadInfo &info) final;
	virtual void unload(Asset asset) final;
};

static SpriteAssetType spriteAssetType;
AssetType *Sprite::assetType = &spriteAssetType;

void SpriteAssetType::load(Asset asset, const AssetLoadInfo &info)
{
	SpriteData &data = g_data;
	sf::MutexGuard mg(data.mutex);

	uint32_t index = asset.assetIndex();
	while (data.sprites.size <= index) {
		data.sprites.push();
	}

	SpriteImp &imp = data.sprites[index];

	int width, height;
	stbi_uc *pixels = stbi_load_from_memory((const stbi_uc*)info.data, (int)info.size, &width, &height, NULL, 4);
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

	imp.info.imageIndex = image.id;
	imp.info.width = width;
	imp.info.height = height;
	imp.info.uvMin = sf::Vec2(0.0f, 0.0f);
	imp.info.uvMax = sf::Vec2(1.0f, 1.0f);
	imp.loaded = true;
}

void SpriteAssetType::unload(Asset asset)
{
}

bool getSpriteInfo(Sprite sprite, SpriteInfo &info)
{
	if (!sprite.valid()) return false;
	sf_assert(sprite.typeIndex() == spriteAssetType.typeIndex);

	SpriteData &data = g_data;

	// Should not be called during asset loading!
	sf_assert(!data.mutex.isLocked());

	uint32_t index = sprite.assetIndex();
	if (index >= data.sprites.size) return false;

	SpriteImp &imp = data.sprites[index];
	if (!imp.loaded) return false;

	info = imp.info;
	return true;
}
