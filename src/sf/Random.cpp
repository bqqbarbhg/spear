#include "Random.h"

namespace sf {

uint32_t Random::nextU32()
{
	uint64_t oldstate = state;
	state = oldstate * 6364136223846793005ULL + inc;
	uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << (((uint32_t)-(int32_t)rot) & 31));
}

float Random::nextFloat()
{
	return (float)((double)nextU32() * 2.3283064365386963e-10); // 1/2^32
}

sf::Vec2 Random::nextVec2()
{
	float x = nextFloat();
	float y = nextFloat();
	return { x, y };
}

sf::Vec3 Random::nextVec3()
{
	float x = nextFloat();
	float y = nextFloat();
	float z = nextFloat();
	return { x, y, z };
}

sf::Vec4 Random::nextVec4()
{
	float x = nextFloat();
	float y = nextFloat();
	float z = nextFloat();
	float w = nextFloat();
	return { x, y, z, w };
}


}
