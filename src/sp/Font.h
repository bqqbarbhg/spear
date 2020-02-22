#pragma once

#include "sf/Base.h"
#include "sf/Vector.h"
#include "sf/Array.h"
#include "ext/sokol/sokol_defs.h"

#include "Asset.h"

namespace sp {

struct TextDraw;

struct FontVertex
{
	sf::Vec2 position;
	sf::Vec2 texCoord;
	uint32_t color;
	uint32_t params;
};

struct FontQuad
{
	FontVertex v[4];
};

struct Font : Asset
{
	static AssetType AssetType;
	using PropType = NoAssetProps;

	// Font must be loaded:
	void getQuads(sf::Array<FontQuad> &quads, const sp::TextDraw &draw, uint32_t color, sf::String text, size_t maxQuads);

	// -- Static API

	static void updateAtlasesForRendering();
	static sg_image getFontAtlasImage();

	// Lifecycle
	static void globalInit();
	static void globalCleanup();
	static void globalUpdate();
};

using FontRef = Ref<Font>;

}
