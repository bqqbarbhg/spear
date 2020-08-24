#pragma once

#include "Asset.h"
#include "sf/Vector.h"
#include "sf/Array.h"
#include "ext/sokol/sokol_defs.h"

namespace sp {

struct Atlas
{
	// Texture extents and handle
	uint32_t width = 0, height = 0;
	sg_image image = { 0 };

	// Record a batch break caused by this atlas,
	// used for optimizing atlases
	void brokeBatch();

	// -- Static API

	// Query a list of currently active atlases
	static void getAtlases(sf::Array<Atlas*> &atlases);
};

struct SpriteProps : AssetProps
{
	// Combine sprites with the same `atlasName` into a single atlas.
	sf::SmallStringBuf<16> atlasName;
	bool tileX = false;
	bool tileY = false;

	virtual uint32_t hash() const final;
	virtual bool equal(const AssetProps &rhs) const final;
	virtual void copyTo(AssetProps *rhs) const final;
};

struct Sprite : Asset
{
	static AssetType SelfType;
	using PropType = SpriteProps;

	// -- Only valid for loaded sprites

	// Atlas and position
	Atlas *atlas = nullptr;
	uint32_t x = 0, y = 0;
	uint32_t width = 0, height = 0;
	uint32_t paddedWidth = 0, paddedHeight = 0;
	float aspect = 0.0f;

	// Cropped quad vertices
	sf::Vec2 minVert, maxVert;

	// UV multiply/add from vertex
	sf::Vec2 vertUvScale, vertUvBias;

	// -- Static API

	// Lifecycle
	static void globalInit();
	static void globalCleanup();
	static void globalUpdate();
};

using SpriteRef = Ref<Sprite>;

}
