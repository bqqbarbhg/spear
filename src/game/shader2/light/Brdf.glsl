#pragma once

#define Brdf_PI 3.14159265359

vec3 F_Schlick(vec3 f0, float VdotH)
{
	float x = clamp(1.0 - VdotH, 0.0, 1.0);
	float x2 = x * x;
	float x4 = x2 * x2;
	return f0 + (asVec3(1.0) - f0) * (x4*x);
}

#if 0

float V_GGX(float NdotL, float NdotV, float alpha2)
{
	float v = NdotL * sqrt(clamp(NdotV * NdotV * (1.0 - alpha2) + alpha2, 0.0, 1.0));
	float l = NdotV * sqrt(clamp(NdotL * NdotL * (1.0 - alpha2) + alpha2, 0.0, 1.0));
	float ggx = v + l;
	return ggx > 0.0 ? 0.5 / ggx : 0.0;
}

float D_GGX(float NdotH, float alpha2)
{
	float x = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
	return alpha2 / (Brdf_PI * x * x);
}

float VD_GGX(float NdotL, float NdotV, float NdotH, float alpha2)
{
	float V = V_GGX(NdotL, NdotV, alpha2);
	float D = D_GGX(NdotH, alpha2);
	return V * D;
}

#else

float VD_GGX(float NdotL, float NdotV, float NdotH, float alpha2)
{
	float ggxV = NdotL * sqrt(clamp(NdotV * NdotV * (1.0 - alpha2) + alpha2, 0.0, 1.0));
	float ggxL = NdotV * sqrt(clamp(NdotL * NdotL * (1.0 - alpha2) + alpha2, 0.0, 1.0));
	float ggxD = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
	float ggx = ggxV + ggxL;
	float denom = (2.0*Brdf_PI) * ggxD * ggxD * ggx;
	return denom > 0.0 ? alpha2 / denom : 0.0;
}

#endif

vec3 BRDF_specularGGX(vec3 cdiff, vec3 f0, float alpha2, float VdotH, float NdotL, float NdotV, float NdotH)
{
	vec3 F = F_Schlick(f0, VdotH);
	float VD = VD_GGX(NdotL, NdotV, NdotH, alpha2);
	return VD * F + (asVec3(1.0) - F) * cdiff * (1.0 / Brdf_PI);
}
