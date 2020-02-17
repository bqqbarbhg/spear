#pragma once

#include "Asset.h"
#include "sf/String.h"
#include "sf/Array.h"

namespace sp {

struct Atlas
{
};

struct SpriteResidency
{
	Atlas *atlas;
	uint32_t x, y;
};

struct Sprite : Asset
{
	static const AssetType AssetType;

	void willBeRendered();
	void getResidency(sf::Array<SpriteResidency> &dst) const;

	// Globals

	static void update();
	static void updateAtlasesForRendering();
};

using SpriteRef = Ref<Sprite>;

}
