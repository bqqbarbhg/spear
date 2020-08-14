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

	virtual uint32_t addSphereArea(AreaGroup group, uint32_t userId, const sf::Sphere &sphere, uint32_t areaFlags) = 0;
	virtual uint32_t addSphereArea(AreaGroup group, uint32_t userId, const sf::Sphere &sphere, const sf::Mat34 &transform, uint32_t areaFlags) = 0;
	virtual void updateSphereArea(uint32_t areaId, const sf::Sphere &sphere) = 0;
	virtual void updateSphereArea(uint32_t areaId, const sf::Sphere &sphere, const sf::Mat34 &transform) = 0;
	virtual void removeSphereArea(uint32_t areaId) = 0;

	virtual void optimize() = 0;

	virtual void queryFrustum(sf::Array<Area> &areas, uint32_t areaFlags, const sf::Frustum &frustum) const = 0;
	virtual void queryFrustumBounds(sf::Array<AreaBounds> &areas, uint32_t areaFlags, const sf::Frustum &frustum) const = 0;
	virtual void castRay(sf::Array<Area> &areas, uint32_t areaFlags, const sf::FastRay &ray, float tMin=0.0f, float tMax=HUGE_VALF) const = 0;

};

}
