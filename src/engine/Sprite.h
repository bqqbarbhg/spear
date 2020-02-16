#pragma once

#include "Asset.h"
#include "sf/Vector.h"

struct SpriteInfo
{
	uint32_t imageIndex;
	sf::Vec2 uvMin, uvMax;
	uint32_t width, height;
};

struct Sprite : Asset
{
	static AssetType *assetType;
	Sprite(Asset a) : Asset(a) { }
};

bool getSpriteInfo(Sprite sprite, SpriteInfo &info);
