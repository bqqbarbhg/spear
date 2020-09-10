#pragma once

#include "sp/Asset.h"
#include "sp/Renderer.h"

namespace cl {

struct EnvmapTexture : sp::Asset
{
	static sp::AssetType SelfType;
	using PropType = sp::NoAssetProps;

	sg_image image;

	static sg_pixel_format pixelFormat;
	static sg_image defaultImage;

	// Lifecycle
	static void globalInit();
	static void globalCleanup();
};

using EnvmapTextureRef = sp::Ref<EnvmapTexture>;

}
