#pragma once

#include "sf/Vector.h"
#include "sf/Matrix.h"

namespace sf {

struct Ray
{
	sf::Vec3 origin;
	sf::Vec3 direction;
};

struct FastRay
{
	union {
		sf::Ray ray;
		struct {
			sf::Vec3 origin;
			sf::Vec3 direction;
		};
	};
	sf::Vec3 rcpDirection;

	sf_forceinline explicit FastRay(const Ray &ray)
		: origin(ray.origin), direction(ray.direction), rcpDirection(sf::Vec3(1.0f) / ray.direction)
	{ }
};

struct Sphere
{
	sf::Vec3 origin;
	float radius;
};

struct Bounds3
{
	sf::Vec3 origin;
	sf::Vec3 extent;

	static Bounds3 minMax(const sf::Vec3 &min, const sf::Vec3 &max) {
		return { (min + max) * 0.5f, (max - min) * 0.5f };
	}

	bool operator==(const Bounds3 &bounds) const { return origin == bounds.origin && extent == bounds.extent; }
	bool operator!=(const Bounds3 &bounds) const { return origin != bounds.origin && extent != bounds.extent; }
};

struct Cube3
{
	sf::Vec3 origin;
	float extent;
};

Ray transformRay(const sf::Mat34 &transform, const Ray &ray);

sf::Bounds3 transformBounds(const sf::Mat34 &transform, const sf::Bounds3 &bounds);
sf::Sphere transformSphere(const sf::Mat34 &transform, const sf::Sphere &sphere);

bool intesersectRay(float &outT, const Ray &ray, const Sphere &sphere, float tMin=0.0f);
bool intesersectRay(float &outT, const Ray &ray, const Bounds3 &bounds, float tMin=0.0f);
bool intesersectRay(float &outT, const Ray &ray, const Bounds3 &bounds, const Mat34 &transform, float tMin=0.0f);
bool intesersectRayObb(float &outT, const Ray &ray, const Mat34 &obb, float tMin=0.0f);

bool intersect(const sf::Bounds3 &a, const sf::Bounds3 &b);
bool intersect(const sf::Bounds3 &a, const sf::Sphere &b);
bool intersect(const sf::Sphere &a, const sf::Sphere &b);

sf_inline bool intersect(const sf::Sphere &a, const sf::Bounds3 &b) {
	return intersect(b, a);
}

Sphere sphereFromBounds3(const Bounds3 &bounds);
Sphere sphereFromBounds3(const Bounds3 &bounds, const sf::Mat34 &transform);
sf::Mat34 obbFromBounds3(const Bounds3 &bounds);
Bounds3 boundsUnion(const Bounds3 &a, const Bounds3 &b);
Sphere sphereUnion(const Sphere &a, const Sphere &b);

float intersectRayFast(const FastRay &ray, const Sphere &sphere, float tMin=0.0f, float tMax=HUGE_VALF);
float intersectRayFast(const FastRay &ray, const Bounds3 &bounds, float tMin=0.0f, float tMax=HUGE_VALF);
float intersectRayFastAabb(const FastRay &ray, const sf::Vec3 &aabbMin, const sf::Vec3 &aabbMax, float tMin=0.0f, float tMax=HUGE_VALF);

}
