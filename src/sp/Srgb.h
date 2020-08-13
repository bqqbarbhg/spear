#pragma once

#include "sf/Vector.h"

namespace sp {

float linearToSrgb(float x);
sf::Vec3 linearToSrgb(const sf::Vec3 &v);

float srgbToLinear(float x);
sf::Vec3 srgbToLinear(const sf::Vec3 &v);

uint8_t linearToSrgbUnorm(float v);
void linearToSrgbUnorm(uint8_t dst[3], const sf::Vec3 &v);

float srgbToLinearUnorm(uint8_t v);
sf::Vec3 srgbToLinearUnorm(const uint8_t v[3]);

}
