#pragma once

#include "sf/Vector.h"
#include "sf/Matrix.h"

namespace sf {

struct Ray
{
	sf::Vec3 origin;
	sf::Vec3 direction;
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
};

Ray transformRay(const sf::Mat34 &transform, const Ray &ray);

bool intesersectRay(float &outT, const Ray &ray, const Sphere &sphere);
bool intesersectRay(float &outT, const Ray &ray, const Bounds3 &bounds);
bool intesersectRay(float &outT, const Ray &ray, const Bounds3 &bounds, const Mat34 &transform);
bool intesersectRayObb(float &outT, const Ray &ray, const Mat34 &obb);

bool intersect(const sf::Bounds3 &a, const sf::Bounds3 &b);
bool intersect(const sf::Bounds3 &a, const sf::Sphere &b);
bool intersect(const sf::Sphere &a, const sf::Sphere &b);

sf_inline bool intersect(const sf::Sphere &a, const sf::Bounds3 &b) {
	return intersect(b, a);
}

Sphere sphereFromBounds3(const Bounds3 &bounds);
Sphere sphereFromBounds3(const Bounds3 &bounds, const sf::Mat34 &transform);
sf::Mat34 obbFromBounds3(const Bounds3 &bounds);
Sphere sphereUnion(const Sphere &a, const Sphere &b);

}
