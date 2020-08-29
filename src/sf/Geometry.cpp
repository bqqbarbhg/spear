#include "Geometry.h"

namespace sf {

Ray transformRay(const sf::Mat34 &transform, const Ray &ray)
{
	Ray r;
	r.origin = sf::transformPoint(transform, ray.origin);
	r.direction = sf::transformDirection(transform, ray.direction);
	return r;
}

sf::Bounds3 transformBounds(const sf::Mat34 &transform, const sf::Bounds3 &bounds)
{
	Bounds3 result;
	result.origin = sf::transformPoint(transform, bounds.origin);
	result.extent = sf::transformDirectionAbs(transform, bounds.extent);
	return result;
}

sf::Sphere transformSphere(const sf::Mat34 &transform, const sf::Sphere &sphere)
{
	Sphere result;
	float scale = sf::sqrt(sf::max(sf::lengthSq(transform.cols[0]), sf::lengthSq(transform.cols[1]), sf::lengthSq(transform.cols[2])));
	result.origin = sf::transformPoint(transform, sphere.origin);
	result.radius = sphere.radius * scale;
	return result;
}

bool intesersectRay(float &outT, const Ray &ray, const Sphere &sphere, float tMin)
{
	sf::Vec3 delta = ray.origin - sphere.origin;
	float a = sf::dot(ray.direction, ray.direction);
	float b = 2.0f * sf::dot(delta, ray.direction);
	float c = sf::dot(delta, delta) - sphere.radius*sphere.radius;
	float radicand = b*b - 4.0f*a*c;
	if (radicand <= 0.0f) return false;
	float root = sf::sqrt(radicand);
	float denom = 0.5f / a;
	if ((-b + root) * denom < tMin) return false;
	outT = (-b - root) * denom;
	return true;
}

bool intesersectRay(float &outT, const Ray &ray, const Bounds3 &bounds, float tMin)
{
	sf::Vec3 rcpDir = sf::Vec3(1.0f) / ray.direction;
	sf::Vec3 relOrigin = bounds.origin - ray.origin;
	sf::Vec3 loT = (relOrigin - bounds.extent) * rcpDir;
	sf::Vec3 hiT = (relOrigin + bounds.extent) * rcpDir;
	sf::Vec3 minT = sf::min(loT, hiT);
	sf::Vec3 maxT = sf::max(loT, hiT);
	float t0 = sf::max(minT.x, minT.y, minT.z);
	float t1 = sf::min(maxT.x, maxT.y, maxT.z);
	if (t0 > t1 || t1 < tMin) return false;
	outT = t0;
	return true;
}

bool intesersectRay(float &outT, const Ray &ray, const Bounds3 &bounds, const Mat34 &transform, float tMin)
{
	sf::Mat34 invTransform = sf::inverse(transform);
	Ray localRay = transformRay(invTransform, ray);
	return intesersectRay(outT, localRay, bounds, tMin);
}

bool intesersectRayObb(float &outT, const Ray &ray, const Mat34 &obb, float tMin)
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
	if (t0 > t1 || t1 < tMin) return false;
	outT = t0;
	return true;
}

bool intersect(const sf::Bounds3 &a, const sf::Bounds3 &b)
{
	sf::Vec3 delta = sf::abs(a.origin - b.origin) - a.extent - b.extent;
	return sf::min(sf::min(delta.x, delta.y), delta.z) <= 0.0f;
}

bool intersect(const sf::Bounds3 &a, const sf::Sphere &b)
{
	sf::Vec3 delta = sf::abs(b.origin - a.origin);
	sf::Vec3 distance = delta - sf::min(delta, a.extent);
	return sf::lengthSq(distance) <= b.radius * b.radius;
}

bool intersect(const sf::Sphere &a, const sf::Sphere &b)
{
	float distSq = sf::lengthSq(a.origin - b.origin);
	float radius = a.radius + b.radius;
	return distSq <= radius * radius;
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

Bounds3 boundsUnion(const Bounds3 &a, const Bounds3 &b)
{
	return Bounds3::minMax(
		sf::min(a.origin - a.extent, b.origin - b.extent),
		sf::max(a.origin + a.extent, b.origin + b.extent));
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

float intersectRayFast(const FastRay &ray, const Sphere &sphere, float tMin, float tMax)
{
	sf::Vec3 delta = ray.origin - sphere.origin;
	float a = sf::dot(ray.direction, ray.direction);
	float b = 2.0f * sf::dot(delta, ray.direction);
	float c = sf::dot(delta, delta) - sphere.radius*sphere.radius;
	float radicand = b*b - 4.0f*a*c;
	if (radicand <= 0.0f) return false;
	float root = sf::sqrt(radicand);
	float denom = 0.5f / a;
	if ((-b + root) * denom < tMin) return false;
	float t = (-b - root) * denom;
	if (t >= tMax) return tMax;
	return sf::max(tMin, t);
}

float intersectRayFast(const FastRay &ray, const Bounds3 &bounds, float tMin, float tMax)
{
	sf::Vec3 relOrigin = bounds.origin - ray.origin;
	sf::Vec3 loT = (relOrigin - bounds.extent) * ray.rcpDirection;
	sf::Vec3 hiT = (relOrigin + bounds.extent) * ray.rcpDirection;
	sf::Vec3 minT = sf::min(loT, hiT);
	sf::Vec3 maxT = sf::max(loT, hiT);
	float t0 = sf::max(minT.x, minT.y, minT.z);
	float t1 = sf::min(maxT.x, maxT.y, maxT.z);
	if (t0 > t1 || t1 < tMin || t0 >= tMax) return tMax;
	return sf::max(t0, tMin);
}

float intersectRayFastAabb(const FastRay &ray, const sf::Vec3 &aabbMin, const sf::Vec3 &aabbMax, float tMin, float tMax)
{
	sf::Vec3 loT = (aabbMin - ray.origin) * ray.rcpDirection;
	sf::Vec3 hiT = (aabbMax - ray.origin) * ray.rcpDirection;
	sf::Vec3 minT = sf::min(loT, hiT);
	sf::Vec3 maxT = sf::max(loT, hiT);
	float t0 = sf::max(minT.x, minT.y, minT.z);
	float t1 = sf::min(maxT.x, maxT.y, maxT.z);
	if (t0 > t1 || t1 < tMin || t0 >= tMax) return tMax;
	return sf::max(t0, tMin);
}

sf::Vec3 closestPointOnRayToPoint(const sf::Ray &ray, const sf::Vec3 &point, float tMin, float tMax)
{
	float t = sf::dot(point - ray.origin, ray.direction) / sf::lengthSq(ray.direction);
	t = sf::clamp(t, tMin, tMax);
	return ray.origin + ray.direction*t;
}

}
