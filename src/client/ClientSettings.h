#pragma once

#include <stdint.h>

struct sg_image_desc;

namespace cl {

struct ClientSettings
{
	enum Preset
	{
		Custom,
		Minimal,
		Low,
		Medium,
		High,
		Ultra,
	};

	bool simpleShading;
	bool useFxaa;
	uint32_t maxResolution;
	bool trilinear;
	uint32_t msaaSamples;
	uint32_t tileMaterialResolution;
	uint32_t meshMaterialResolution;
	uint32_t maxAnisotropy;
	uint32_t diffuseProbeSlices;
	uint32_t diffuseProbeUpdateCount;
	uint32_t diffuseProbeResolution;
	float diffuseProbeDistance;
	bool diffuseProbeSmallFloat;

	uint32_t shadowCacheResolution;
	uint32_t shadowCacheSlices;
	uint32_t shadowDepthResolution;

	uint32_t lightQuality;
};

void initDefaultSettings(ClientSettings &settings, ClientSettings::Preset preset);
void initSampler(sg_image_desc &desc, const ClientSettings &settings);

extern ClientSettings g_settings;

}
