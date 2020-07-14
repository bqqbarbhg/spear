#pragma once

#include "util/Srgb.glsl"

vec4 tonemap(vec3 v)
{
	float exposure = 2.0;
	v *= exposure;
	vec3 x = v / (1.0 + v);
	x = linearToSrgb(x);
	return vec4(x, dot(x, vec3(0.299, 0.587, 0.114)));
}
