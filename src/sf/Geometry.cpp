#include "Geometry.h"

namespace sf {

Ray transformRay(const sf::Mat34 &transform, const Ray &ray)
{
	Ray r;
	r.origin = sf::transformPoint(transform, ray.origin);
	r.direction = sf::transformDirection(transform, ray.direction);
	return r;
}

bool intesersectRay(float &outT, const Ray &ray, const Sphere &sphere)
{
	sf::Vec3 delta = ray.origin - sphere.origin;
	float a = sf::dot(ray.direction, ray.direction);
	float b = 2.0f * sf::dot(delta, ray.direction);
	float c = sf::dot(delta, delta) - sphere.radius*sphere.radius;
	float radicand = b*b - 4.0f*a*c;
	if (radicand <= 0.0f) return false;
	float root = sf::sqrt(radicand);
	float denom = 0.5f / root;
	outT = (-b - root) * denom;
	return true;
}

bool intesersectRay(float &outT, const Ray &ray, const Bounds3 &bounds)
{
	sf::Vec3 rcpDir = sf::Vec3(1.0f) / ray.direction;
	sf::Vec3 relOrigin = bounds.origin - ray.origin;
	sf::Vec3 loT = (relOrigin - bounds.extent) * rcpDir;
	sf::Vec3 hiT = (relOrigin + bounds.extent) * rcpDir;
	sf::Vec3 minT = sf::min(loT, hiT);
	sf::Vec3 maxT = sf::max(loT, hiT);
	float t0 = sf::max(minT.x, minT.y, minT.z);
	float t1 = sf::min(maxT.x, maxT.y, maxT.z);
	if (t0 >= t1) return false;
	outT = t0;
	return true;
}

bool intesersectRay(float &outT, const Ray &ray, const Bounds3 &bounds, const Mat34 &transform)
{
	sf::Mat34 invTransform = sf::inverse(transform);
	Ray localRay = transformRay(invTransform, ray);
	return intesersectRay(outT, localRay, bounds);
}

bool intesersectRayObb(float &outT, const Ray &ray, const Mat34 &obb)
{
	sf::Mat34 invTransform = sf::inverse(obb);
	Ray localRay = transformRay(invTransform, ray);
	sf::Vec3 rcpDir = sf::Vec3(1.0f) / localRay.direction;
	sf::Vec3 relOrigin = -localRay.origin;
	sf::Vec3 loT = (relOrigin - sf::Vec3(1.0f)) * rcpDir;
	sf::Vec3 hiT = (relOrigin + sf::Vec3(1.0f)) * rcpDir;
	sf::Vec3 minT = sf::min(loT, hiT);
	sf::Vec3 maxT = sf::max(loT, hiT);
	float t0 = sf::max(minT.x, minT.y, minT.z);
	float t1 = sf::min(maxT.x, maxT.y, maxT.z);
	if (t0 >= t1) return false;
	outT = t0;
	return true;
}

Sphere sphereFromBounds3(const Bounds3 &bounds)
{
	Sphere s;
	s.origin = bounds.origin;
	s.radius = sf::length(bounds.extent);
	return s;
}

Sphere sphereFromBounds3(const Bounds3 &bounds, const sf::Mat34 &transform)
{
	Sphere s;
	s.origin = sf::transformPoint(transform, bounds.origin);
	s.radius = sf::length(sf::transformDirection(transform, bounds.extent));
	return s;
}

sf::Mat34 obbFromBounds3(const Bounds3 &bounds)
{
	return sf::mat::translate(bounds.origin) * sf::mat::scale(bounds.extent);
}

Sphere sphereUnion(const Sphere &a, const Sphere &b)
{
	sf::Vec3 delta = b.origin - a.origin;
	float dist = sf::length(delta);
	if (b.radius < 0.0f || a.radius >= dist + b.radius) return a;
	if (a.radius < 0.0f || b.radius >= dist + a.radius) return b;
	Sphere s;
	s.radius = (dist + a.radius + b.radius) * 0.5f;
	s.origin = a.origin + delta * ((s.radius - a.radius) / dist);
	return s;
}

}
