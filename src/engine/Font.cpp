#include "Font.h"

#include "sf/Mutex.h"
#include "sf/Array.h"

struct FontImp
{
	sf::Array<char> data;
	bool loaded = false;
};

struct FontData
{
	sf::Mutex mutex;
	sf::Array<FontImp> fonts;
};
static FontData g_data;

struct FontAssetType : AssetType
{
	virtual void load(Asset asset, const AssetLoadInfo &info) final;
	virtual void unload(Asset asset) final;
};

static FontAssetType fontAssetType;
AssetType *Font::assetType = &fontAssetType;

void FontAssetType::load(Asset asset, const AssetLoadInfo &info)
{
	FontData &data = g_data;
	sf::MutexGuard mg(data.mutex);

	uint32_t index = asset.assetIndex();
	while (data.fonts.size <= index) {
		data.fonts.push();
	}

	FontImp &imp = data.fonts[index];
	imp.data.resizeUninit(info.size);
	memcpy(imp.data.data, info.data, info.size);

	imp.loaded = true;
}

void FontAssetType::unload(Asset asset)
{
	FontData &data = g_data;
	sf::MutexGuard mg(data.mutex);

	FontImp &imp = data.fonts[asset.assetIndex()];
	imp.data.clear();
	imp.data.trim();
}

bool getFontInfo(Font font, FontInfo &info)
{
	if (!font.valid()) return false;
	sf_assert(font.typeIndex() == fontAssetType.typeIndex);

	FontData &data = g_data;

	// Should not be called during asset loading!
	sf_assert(!data.mutex.isLocked());

	uint32_t index = font.assetIndex();
	if (index >= data.fonts.size) return false;

	FontImp &imp = data.fonts[index];
	if (!imp.loaded) return false;

	info.data = imp.data.data;
	info.dataSize = imp.data.size;
	return true;
}

