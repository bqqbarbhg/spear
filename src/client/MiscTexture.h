#pragma once

#include "sp/Asset.h"
#include "sp/Renderer.h"

namespace cl {

struct MiscTextureProps : sp::AssetProps
{
	// Combine sprites with the same `atlasName` into a single atlas.
	sg_filter minFilter = SG_FILTER_LINEAR;
	sg_filter magFilter = SG_FILTER_LINEAR;

	virtual uint32_t hash() const final;
	virtual bool equal(const sp::AssetProps &rhs) const final;
	virtual void copyTo(sp::AssetProps *rhs) const final;
};


struct MiscTexture : sp::Asset
{
	static sp::AssetType SelfType;
	using PropType = MiscTextureProps;

	sg_image image;
	sg_pixel_format pixelFormat = SG_PIXELFORMAT_NONE;
};

using MiscTextureRef = sp::Ref<MiscTexture>;

}
