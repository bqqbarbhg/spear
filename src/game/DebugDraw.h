#pragma once

#include "sf/Vector.h"
#include "sf/Geometry.h"

struct DebugLine
{
	sf::Vec3 a, b;
	sf::Vec3 color;
};

struct DebugSphere
{
	sf::Mat34 transform;
	sf::Vec3 color;
};

struct DebugDrawData
{
	sf::Slice<DebugLine> lines;
	sf::Slice<DebugSphere> spheres;
};

void debugDrawPushTransform(const sf::Mat34 &transform);
void debugDrawPopTransform();

void debugDrawLine(const DebugLine &line);
void debugDrawLine(const sf::Vec3 &a, const sf::Vec3 &b, const sf::Vec3 &color=sf::Vec3(1.0f));
void debugDrawBox(const sf::Mat34 &transform, const sf::Vec3 &color=sf::Vec3(1.0f));
void debugDrawBox(const sf::Bounds3 &bounds, const sf::Vec3 &color=sf::Vec3(1.0f));
void debugDrawSphere(const sf::Mat34 &transform, const sf::Vec3 &color=sf::Vec3(1.0f));
void debugDrawSphere(const sf::Vec3 &origin, float radius, const sf::Vec3 &color=sf::Vec3(1.0f));
void debugDrawSphere(const sf::Sphere &sphere, const sf::Vec3 &color=sf::Vec3(1.0f));

void debugDrawFlipBuffers();
DebugDrawData debugDrawGetData();
