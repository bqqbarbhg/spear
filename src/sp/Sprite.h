#pragma once

#include "Asset.h"
#include "sf/String.h"
#include "sf/Array.h"
#include "sf/Vector.h"

namespace sp {

struct Atlas
{
	uint32_t width = 0, height = 0;

	void brokeBatch();
	uint32_t getTexture();

	static void getAtlases(sf::Array<Atlas*> &atlases);
};

struct Sprite : Asset
{
	static const AssetType AssetType;

	// Only valid for loaded sprites!
	Atlas *atlas = nullptr;
	uint32_t x = 0, y = 0;
	uint32_t width = 0, height = 0;
	sf::Vec2 minVert, maxVert;

	// Globals

	static void update();
};

using SpriteRef = Ref<Sprite>;

}
