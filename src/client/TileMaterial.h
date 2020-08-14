#pragma once

#include "sp/Asset.h"
#include "sf/Vector.h"
#include "sf/Array.h"
#include "ext/sokol/sokol_defs.h"
#include "MeshMaterial.h"

namespace cl {

struct TileMaterial : sp::Asset
{
	static sp::AssetType SelfType;
	using PropType = sp::NoAssetProps;

	sf::Vec2 uvBase, uvScale;

	// -- Static API

	static sg_image getAtlasImage(MaterialTexture texture);
	static uint32_t getAtlasPixelFormat(MaterialTexture texture);

	// Lifecycle
	static void globalInit();
	static void globalCleanup();
};

using TileMaterialRef = sp::Ref<TileMaterial>;

}
