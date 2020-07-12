#pragma once

#include "ext/sokol/sokol_gfx.h"
#include "sp/Asset.h"

namespace cl {

// TODO: Move to sp::
enum class MaterialTexture
{
	Albedo,
	Normal,
	Mask,

	Count,
};

const char *getPixelFormatSuffix(sg_pixel_format format);

struct MeshMaterial : sp::Asset
{
	static sp::AssetType SelfType;
	using PropType = sp::NoAssetProps;

	sg_image images[(uint32_t)MaterialTexture::Count] = { };

	static sg_pixel_format materialFormats[(uint32_t)MaterialTexture::Count];
	static sg_image defaultImages[(uint32_t)MaterialTexture::Count];

	sf_forceinline sg_image getImage(MaterialTexture tex) {
		return images[(uint32_t)tex].id ? images[(uint32_t)tex] : defaultImages[(uint32_t)tex];
	}

	// Lifecycle
	static void globalInit();
	static void globalCleanup();
};

using MeshMaterialRef = sp::Ref<MeshMaterial>;

}
