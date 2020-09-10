#include "util/Defines.glsl"

layout(location=0) attribute vec3 a_position;
layout(location=1) attribute vec2 a_uv;
layout(location=2) attribute vec3 a_normal;
layout(location=3) attribute vec4 a_tangent;
#if SP_GLSL
	layout(location=4) attribute vec4 a_indices;
#else
	layout(location=4) attribute uvec4 a_indices;
#endif
layout(location=5) attribute vec4 a_weights;

varying vec3 v_position;
varying vec3 v_normal;
varying vec3 v_tangent;
varying vec3 v_bitangent;
varying vec2 v_uv;

#ifdef SP_VS

uniform SkinTransform
{
	mat4 worldToClip;
};

uniform Bones
{
	vec4 bones[SP_MAX_BONES * 3];
};

void main()
{
#if SOKOL_GLSL
	ivec4 ix = ivec4(a_indices * 3.0);
#else
	ivec4 ix = ivec4(a_indices) * 3;
#endif
    vec4 weights = a_weights;
	vec4 row0 = bones[ix.x + 0] * weights.x;
	vec4 row1 = bones[ix.x + 1] * weights.x;
	vec4 row2 = bones[ix.x + 2] * weights.x;
	row0 += bones[ix.y + 0] * weights.y;
	row1 += bones[ix.y + 1] * weights.y;
	row2 += bones[ix.y + 2] * weights.y;
	row0 += bones[ix.z + 0] * weights.z;
	row1 += bones[ix.z + 1] * weights.z;
	row2 += bones[ix.z + 2] * weights.z;
	row0 += bones[ix.w + 0] * weights.w;
	row1 += bones[ix.w + 1] * weights.w;
	row2 += bones[ix.w + 2] * weights.w;

    vec4 pos = vec4(a_position, 1.0);
	vec3 p = vec3(
		dot(row0, pos),
		dot(row1, pos),
		dot(row2, pos));

    vec4 nrm = vec4(a_normal, 0.0);
	vec3 n = vec3(
		dot(row0, nrm),
		dot(row1, nrm),
		dot(row2, nrm));

    vec4 tng = vec4(a_tangent.xyz, 0.0);
	vec3 t = vec3(
		dot(row0, tng),
		dot(row1, tng),
		dot(row2, tng));

	v_position = p;
    gl_Position = mul(vec4(p, 1.0), worldToClip);
	v_normal = normalize(n);
	v_tangent = normalize(t);
	v_bitangent = a_tangent.w * cross(v_normal, v_tangent); 
	v_uv = a_uv;
}

#endif

#ifdef SP_FS

#pragma permutation SP_SHADOWGRID_USE_ARRAY 2
#pragma permutation SP_NORMALMAP_REMAP 2

#define MAX_LIGHTS 16

uniform Pixel
{
	float numLightsF;
	vec3 cameraPosition;
	vec4 pointLightData[MAX_LIGHTS*SP_POINTLIGHT_DATA_SIZE];
};

#include "light/PointLight.glsl"
#include "light/IBL.glsl"
#include "util/NormalMap.glsl"
#include "util/Tonemap.glsl"

out vec4 o_color;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D maskTexture;

void main()
{
	vec3 result = asVec3(0.0);
	int end = int(numLightsF) * SP_POINTLIGHT_DATA_SIZE;

	vec2 matNormal = sampleNormalMap(normalTexture, v_uv);
	float matNormalY = sqrt(clamp(1.0 - dot(matNormal, matNormal), 0.0, 1.0));

	vec3 matAlbedo = texture(albedoTexture, v_uv).xyz;
	vec4 matMask = texture(maskTexture, v_uv);
	
	vec3 P = v_position;
	vec3 N = normalize(matNormal.x * v_tangent + matNormal.y * v_bitangent + matNormalY * v_normal);
	vec3 V = normalize(cameraPosition - v_position);
	vec3 cdiff = matAlbedo.xyz * (1.0 - matMask.x);
	vec3 f0 = lerp(asVec3(0.03), matAlbedo, matMask.x);
	float alpha = matMask.w*matMask.w;
	float alpha2 = alpha*alpha;

	result += matAlbedo * matMask.z;

	result += evaluateIBL(N, V, cdiff, f0, alpha) * matMask.y;

	for (int base = 0; base < end; base += SP_POINTLIGHT_DATA_SIZE) {
		result += evaluatePointLight(P, N, V, cdiff, f0, alpha2, base);
	}

	o_color = tonemap(result);
}

#endif

