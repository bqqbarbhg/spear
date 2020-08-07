#pragma once

#include "server/ServerState.h"
#include "client/EntityState.h"
#include "sf/Geometry.h"
#include "ext/sokol/sokol_defs.h"

namespace cl {

struct PointLight
{
	sf::Vec3 position;
	sf::Vec3 color;
	float radius;
	sf::Vec3 shadowMul;
	sf::Vec3 shadowBias;
};

struct LightState
{
	static sf::Box<LightState> create();

	void addPointLight(uint32_t entityId, const sv::PointLightComponent &c);
	void updateEntity(uint32_t entityId, const EntityState &state, uint32_t updateMask);
	void removeEntity(uint32_t entityId);

	void renderDirtyShadowMaps(uint32_t maxUpdates);

	void updatePointLightVisibility();
	void queryVisiblePointLights(const sf::Bounds3 &bounds, uint32_t maxPointLights, sf::Array<PointLight> &pointLights);

	sg_image getShadowCache() const;
};

}
