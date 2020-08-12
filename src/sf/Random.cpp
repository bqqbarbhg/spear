#include "Random.h"

// TODO: Implement this
#include <random>

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

void getSecureRandom(void *dst, size_t size)
{
	std::random_device dev;
	std::uniform_int_distribution<uint32_t> dist;

	char *ptr = (char*)dst;

	uint32_t buf[32];
	while (size > 0) {
		uint32_t chunkSize = (uint32_t)sf::min(size, sizeof(buf));
		uint32_t words = (chunkSize + 3) / 4;

		for (uint32_t i = 0; i < words; i++) {
			buf[i] = dist(dev);
		}
		memcpy(ptr, buf, chunkSize);

		ptr += chunkSize;
		size -= chunkSize;
	}

}

}
