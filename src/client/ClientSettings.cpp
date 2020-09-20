#include "ClientSettings.h"

#include "ext/sokol/sokol_gfx.h"

namespace cl {

void initDefaultSettings(ClientSettings &settings, ClientSettings::Preset preset)
{
	if (preset == ClientSettings::Minimal) {
		settings.simpleShading = true;
		settings.msaaSamples = 1;
		settings.useFxaa = false;
		settings.maxResolution = 540;
		settings.tileMaterialResolution = 128;
		settings.meshMaterialResolution = 128;
		settings.trilinear = false;
		settings.maxAnisotropy = 0;
		settings.diffuseProbeSlices = 1;
		settings.diffuseProbeUpdateCount = 1;
		settings.diffuseProbeResolution = 48;
		settings.diffuseProbeDistance = 1.0f;
		settings.diffuseProbeSmallFloat = true;
		settings.shadowCacheResolution = 32;
		settings.shadowCacheSlices = 8;
		settings.shadowDepthResolution = 64;
		settings.lightQuality = 0;
	} else if (preset == ClientSettings::Low) {
		settings.simpleShading = false;
		settings.msaaSamples = 1;
		settings.useFxaa = true;
		settings.maxResolution = 720;
		settings.tileMaterialResolution = 256;
		settings.meshMaterialResolution = 512;
		settings.trilinear = false;
		settings.maxAnisotropy = 0;
		settings.diffuseProbeSlices = 2;
		settings.diffuseProbeUpdateCount = 1;
		settings.diffuseProbeResolution = 48;
		settings.diffuseProbeDistance = 1.0f;
		settings.diffuseProbeSmallFloat = true;
		settings.shadowCacheResolution = 64;
		settings.shadowCacheSlices = 8;
		settings.shadowDepthResolution = 128;
		settings.lightQuality = 2;
	} else if (preset == ClientSettings::Medium) {
		settings.simpleShading = false;
		settings.msaaSamples = 1;
		settings.useFxaa = true;
		settings.maxResolution = 1080;
		settings.tileMaterialResolution = 512;
		settings.meshMaterialResolution = 512;
		settings.trilinear = true;
		settings.maxAnisotropy = 2;
		settings.diffuseProbeSlices = 2;
		settings.diffuseProbeUpdateCount = 2;
		settings.diffuseProbeResolution = 64;
		settings.diffuseProbeDistance = 1.0f;
		settings.diffuseProbeSmallFloat = true;
		settings.shadowCacheResolution = 128;
		settings.shadowCacheSlices = 8;
		settings.shadowDepthResolution = 256;
		settings.lightQuality = 3;
	} else if (preset == ClientSettings::High) {
		settings.simpleShading = false;
		settings.msaaSamples = 4;
		settings.useFxaa = false;
		settings.maxResolution = 1440;
		settings.tileMaterialResolution = 1024;
		settings.meshMaterialResolution = 1024;
		settings.trilinear = true;
		settings.maxAnisotropy = 4;
		settings.diffuseProbeSlices = 3;
		settings.diffuseProbeUpdateCount = 3;
		settings.diffuseProbeResolution = 128;
		settings.diffuseProbeDistance = 0.5f;
		settings.diffuseProbeSmallFloat = false;
		settings.shadowCacheResolution = 256;
		settings.shadowCacheSlices = 8;
		settings.shadowDepthResolution = 512;
		settings.lightQuality = 4;
	} else if (preset == ClientSettings::Ultra) {
		settings.simpleShading = false;
		settings.msaaSamples = 8;
		settings.useFxaa = false;
		settings.maxResolution = UINT32_MAX;
		settings.tileMaterialResolution = 1024;
		settings.meshMaterialResolution = 1024;
		settings.trilinear = true;
		settings.maxAnisotropy = 8;
		settings.diffuseProbeSlices = 4;
		settings.diffuseProbeUpdateCount = 3;
		settings.diffuseProbeResolution = 128;
		settings.diffuseProbeDistance = 0.5f;
		settings.diffuseProbeSmallFloat = false;
		settings.shadowCacheResolution = 256;
		settings.shadowCacheSlices = 8;
		settings.shadowDepthResolution = 512;
		settings.lightQuality = 5;
	}
}

void initSampler(sg_image_desc &desc, const ClientSettings &settings)
{
	desc.mag_filter = SG_FILTER_LINEAR;
	desc.min_filter = settings.trilinear ? SG_FILTER_LINEAR_MIPMAP_LINEAR : SG_FILTER_LINEAR_MIPMAP_NEAREST;
	desc.max_anisotropy = settings.maxAnisotropy;
}

ClientSettings g_settings;

}
