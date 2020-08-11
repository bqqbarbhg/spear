#pragma once

#include "sf/Vector.h"

namespace sv {

int32_t fixedSin360(int32_t angle);
int32_t fixedCos360(int32_t angle);
int32_t fixedSin(int32_t angle);
int32_t fixedCos(int32_t angle);

sf_inline int32_t fixedMul(int32_t a, int32_t b) {
	return (int32_t)(((int64_t)a * (int64_t)b) >> 16);
}

sf_inline int32_t fixedDot(const sf::Vec2i &a, const sf::Vec2i &b) {
	return fixedMul(a.x, b.x) + fixedMul(a.y, b.y);
}

}
