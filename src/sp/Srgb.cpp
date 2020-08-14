#include "Srgb.h"

namespace sp {

sf_inline float unormToFloat(uint8_t v)
{
	return (float)v * (1.0f / 255.0f);
}

sf_inline uint8_t floatToUnorm(float v)
{
	return (uint8_t)(v * 255.9f);
}

float linearToSrgb(float x)
{
	x = sf::clamp(x, 0.0f, 1.0f);
	if (x <= 0.00031308f)
		return 12.92f * x;
	else
		return 1.055f*powf(x, (1.0f / 2.4f)) - 0.055f;
}

sf::Vec3 linearToSrgb(const sf::Vec3 &v)
{
	return sf::Vec3(linearToSrgb(v.x), linearToSrgb(v.y), linearToSrgb(v.z));
}

float srgbToLinear(float x)
{
	x = sf::clamp(x, 0.0f, 1.0f);
	if (x <= 0.04045f)
		return 0.0773993808f * x;
	else
		return powf(x*0.947867298578f+0.0521327014218f, 2.4f);
}

sf::Vec3 srgbToLinear(const sf::Vec3 &v)
{
	return sf::Vec3(srgbToLinear(v.x), srgbToLinear(v.y), srgbToLinear(v.z));
}

uint8_t linearToSrgbUnorm(float v)
{
	return floatToUnorm(linearToSrgb(v));
}

void linearToSrgbUnorm(uint8_t dst[3], const sf::Vec3 &v)
{
	dst[0] = floatToUnorm(linearToSrgb(v.x));
	dst[1] = floatToUnorm(linearToSrgb(v.y));
	dst[2] = floatToUnorm(linearToSrgb(v.z));
}

float srgbToLinearUnorm(uint8_t v)
{
	return srgbToLinear(unormToFloat(v));
}

sf::Vec3 srgbToLinearUnorm(const uint8_t v[3])
{
	return sf::Vec3(
		srgbToLinear(unormToFloat(v[0])),
		srgbToLinear(unormToFloat(v[1])),
		srgbToLinear(unormToFloat(v[2])));
}

}
