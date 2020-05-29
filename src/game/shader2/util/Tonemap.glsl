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

vec4 tonemap(vec3 v)
{
	vec3 x = v / (1.0 + v);
	x = linearToSrgb(x);
	return vec4(x, dot(x, vec3(0.299, 0.587, 0.114)));
}
