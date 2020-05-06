#pragma once

#include "sf/Vector.h"

namespace sf {

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

}
