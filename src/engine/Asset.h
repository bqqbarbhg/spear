#pragma once

#include "sf/Base.h"
#include "sf/String.h"

struct Asset
{
	uint32_t id;

	Asset()
		: id(0)
	{
	}

	Asset(uint32_t typeIndex, uint32_t assetIndex)
		: id(typeIndex << 24u | assetIndex)
	{
	}

	bool valid() const { return id != 0; }
	uint32_t typeIndex() const { return id >> 24u; }
	uint32_t assetIndex() const { return id & ((1 << 24u) - 1); }
};

struct AssetLoadInfo
{
	sf::String name;
	const void *data;
	size_t size;
};

struct AssetType
{
	uint32_t typeIndex = 0;

	virtual void load(Asset asset, const AssetLoadInfo &info) = 0;
	virtual void unload(Asset asset) = 0;
};

Asset loadAssetImp(sf::String name, AssetType *type);

template <typename T>
T loadAsset(const char *name)
{
	Asset asset = loadAssetImp(sf::String(name), T::assetType);
	return T(asset);
}

void setupAssets();
void updateAssets();
