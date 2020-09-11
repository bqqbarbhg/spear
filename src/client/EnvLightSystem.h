#pragma once

#include "client/System.h"
#include "ext/sokol/sokol_defs.h"

namespace cl {

struct EnvLightAltas
{
	sg_image image;
	sf::Vec4 worldMad;
};

struct EnvLightSystem : System
{
	static sf::Box<EnvLightSystem> create();

	virtual void renderEnvmap(Systems &systems) = 0;

	virtual EnvLightAltas getEnvLightAtlas() const = 0;

	virtual sg_image getDebugLightingImage() const = 0;
};

}
