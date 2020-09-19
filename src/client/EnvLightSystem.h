#pragma once

#include "client/System.h"
#include "ext/sokol/sokol_defs.h"

namespace cl {

struct EnvLightAltas
{
	sg_image image;
	sf::Vec4 worldMad;
};

struct EnvVisualizeSphereOpts
{
	float specular = 0.0f;
	float radius = 0.1f;
};

struct EnvLightSystem : System
{
	static sf::Box<EnvLightSystem> create();

	virtual void renderEnvmap(Systems &systems) = 0;

	virtual void renderEnvmapDebug(Systems &systems, const RenderArgs &renderArgs) = 0;
	virtual void compositeEnvmapDebug() = 0;

	virtual void renderEnvmapDebugSpheres(const LightSystem *lightSystem, const RenderArgs &renderArgs, const EnvVisualizeSphereOpts &opts) = 0;

	virtual void setIblEnabled(bool enabled) = 0;

	virtual EnvLightAltas getEnvLightAtlas() const = 0;

	virtual sg_image getDebugLightingImage() const = 0;
};

}
