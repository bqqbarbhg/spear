#pragma once

#include "Asset.h"
#include "sf/String.h"
#include "sf/Array.h"

namespace sp {

struct Atlas
{
	uint32_t width = 0, height = 0;

	uint32_t getTexture();

	static void getAtlases(sf::Array<Atlas*> &atlases);
};

struct SpriteResidency
{
	Atlas *atlas;
	uint32_t x, y;
};

struct Sprite : Asset
{
	static const AssetType AssetType;

	// Only valid for loaded sprites!
	uint32_t width = 0, height = 0;

	void willBeRendered();
	void getResidency(sf::Array<SpriteResidency> &dst) const;
	bool isResident() const;

	// Globals

	static void update();
	static void updateAtlasesForRendering();
};

using SpriteRef = Ref<Sprite>;

}
