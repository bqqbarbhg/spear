#pragma once

#ifndef SP_SHADOWGRID_USE_ARRAY
    #error "Permutation SP_SHADOWGRID_USE_ARRAY not defined"
#endif

#if SP_SHADOWGRID_USE_ARRAY

uniform sampler2DArray shadowGridArray;

float evaluateShadowGrid(vec3 p)
{
	float ly = p.y * 8.0;
	float l0 = clamp(floor(ly), 0.0, 7.0);
	float l1 = min(l0 + 1.0, 7.0);
	float t = ly - l0;
	float s0 = textureLod(shadowGridArray, vec3(p.xz, l0), 0.0).x;
	float s1 = textureLod(shadowGridArray, vec3(p.xz, l1), 0.0).x;
	return lerp(s0, s1, t);
}

#else

uniform sampler3D shadowGrid3D;

#define evaluateShadowGrid(p) textureLod(shadowGrid3D, (p).xzy, 0.0).x

#endif
