#pragma once

#include "sf/Vector.h"

namespace sf {

struct Random
{
	uint64_t state, inc;

	Random(uint64_t state=1, uint64_t inc=1) : state(state), inc(inc|1) { }

	uint32_t nextU32();
	float nextFloat();
	sf::Vec2 nextVec2();
	sf::Vec3 nextVec3();
	sf::Vec4 nextVec4();
};

}
