#pragma once

#include "Asset.h"
#include "sf/Vector.h"

struct FontInfo
{
	const void *data;
	size_t dataSize;
};

struct Font : Asset
{
	static AssetType *assetType;
	Font(Asset a) : Asset(a) { }
};

bool getFontInfo(Font font, FontInfo &info);
