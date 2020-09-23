#pragma once

#include "client/System.h"

#include "server/ServerState.h"
#include "sf/Box.h"
#include "ext/sokol/sokol_defs.h"

namespace cl {

struct VisFogImage
{
	sf::Vec4 worldMad;
	sg_image image;
};

struct VisFogSystem : System
{
	static sf::Box<VisFogSystem> create();

	virtual void updateVisibility(const sv::VisibleUpdateEvent &e) = 0;

	virtual void updateTexture(float dt) = 0;

	virtual VisFogImage getVisFogImage() const = 0;
};

}
