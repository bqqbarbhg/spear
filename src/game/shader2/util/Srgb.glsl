#pragma once

float linearToSrgb(float x)
{
	x = clamp(x, 0.0, 1.0);
	if (x <= 0.00031308)
		return 12.92 * x;
	else
		return 1.055*pow(x,(1.0 / 2.4) ) - 0.055;
}

vec3 linearToSrgb(vec3 v)
{
	return vec3(linearToSrgb(v.x), linearToSrgb(v.y), linearToSrgb(v.z));
}

float srgbToLinear(float x)
{
	x = clamp(x, 0.0, 1.0);
	if (x <= 0.04045)
		return 0.0773993808 * x;
	else
		return pow(x*0.947867298578+0.0521327014218, 2.4);
}

vec3 srgbToLinear(vec3 v)
{
	return vec3(srgbToLinear(v.x), srgbToLinear(v.y), srgbToLinear(v.z));
}
