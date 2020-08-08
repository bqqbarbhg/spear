#pragma once

#include "server/ServerState.h"
#include "sf/Geometry.h"
#include "ext/sokol/sokol_defs.h"

namespace cl {

struct VisualTransform;

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
	void removeEntity(uint32_t entityId);

	void updateEntityTransform(uint32_t entityId, const VisualTransform &transform, const sf::Mat34 &matrix);

	void renderDirtyShadowMaps(uint32_t maxUpdates);

	void updateVisibility();
	void queryVisiblePointLights(const sf::Bounds3 &bounds, uint32_t maxPointLights, sf::Array<PointLight> &pointLights);

	sg_image getShadowCache() const;
};

}
