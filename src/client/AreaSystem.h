#pragma once

#include "client/System.h"
#include "sf/Geometry.h"

namespace sf { struct Frustum; }

namespace cl {

struct AreaSystem : System
{
	static sf::Box<AreaSystem> create();

	virtual uint32_t addBoxArea(AreaGroup group, uint32_t userId, const sf::Bounds3 &bounds, uint32_t areaFlags) = 0;
	virtual uint32_t addBoxArea(AreaGroup group, uint32_t userId, const sf::Bounds3 &bounds, const sf::Mat34 &transform, uint32_t areaFlags) = 0;
	virtual void updateBoxArea(uint32_t areaId, const sf::Bounds3 &bounds) = 0;
	virtual void updateBoxArea(uint32_t areaId, const sf::Bounds3 &bounds, const sf::Mat34 &transform) = 0;
	virtual void removeBoxArea(uint32_t areaId) = 0;

	virtual void optimize() = 0;

	virtual void queryFrustum(sf::Array<Area> &areas, uint32_t areaFlags, const sf::Frustum &frustum) const = 0;
	virtual void castRay(sf::Array<Area> &areas, uint32_t areaFlags, const sf::Ray &ray, float tMin=0.0f, float tMax=HUGE_VALF) const = 0;
};

}
