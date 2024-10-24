
#include "util/Defines.glsl"

layout(location=0) attribute vec3 a_position;
layout(location=1) attribute vec3 a_normal;
layout(location=2) attribute vec4 a_tangent;
layout(location=3) attribute vec2 a_uv;
layout(location=4) attribute vec4 a_tint;

#pragma permutation SP_SAMPLE_SHADING 2

#if SP_SAMPLE_SHADING && defined(SP_HLSL)
	#define PER_SAMPLE sample
#else
	#define PER_SAMPLE
#endif

PER_SAMPLE varying vec3 v_position;
PER_SAMPLE varying vec3 v_normal;
PER_SAMPLE varying vec3 v_tangent;
PER_SAMPLE varying vec3 v_bitangent;
PER_SAMPLE varying vec2 v_uv;
PER_SAMPLE varying vec4 v_tint;

#ifdef SP_VS

#include "util/Srgb.glsl"

uniform Transform
{
	mat4 worldToClip;
};

vec3 unpackUnorm10ToSnormVec3(vec3 v) { return v * (1024.0/511.0) - asVec3(1.0); }

void main()
{
	v_position = a_position;
    gl_Position = mul(vec4(a_position, 1.0), worldToClip);
	v_normal = unpackUnorm10ToSnormVec3(a_normal);
	v_tangent = unpackUnorm10ToSnormVec3(a_tangent.xyz);
	v_bitangent = (a_tangent.w * 2.0 - 1.0) * cross(v_normal, v_tangent);
	v_uv = a_uv;
	v_tint.xyz = srgbToLinear(a_tint.xyz);
	v_tint.w = 1.0;
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
	vec4 diffuseEnvmapMad;
	vec4 visFogMad;
	vec4 pointLightData[MAX_LIGHTS*SP_POINTLIGHT_DATA_SIZE];
};

#include "light/PointLight.glsl"
#include "light/IBL.glsl"
#include "light/VisFog.glsl"
#include "util/NormalMap.glsl"
#include "util/Tonemap.glsl"

out vec4 o_color;

uniform sampler2D albedoAtlas;
uniform sampler2D normalAtlas;
uniform sampler2D maskAtlas;

void main()
{
	vec3 result = asVec3(0.0);

	vec2 matNormal = sampleNormalMap(normalAtlas, v_uv);
	float matNormalY = sqrt(clamp(1.0 - dot(matNormal, matNormal), 0.0, 1.0));

	vec3 matAlbedo = texture(albedoAtlas, v_uv).xyz * v_tint.xyz;
	vec4 matMask = texture(maskAtlas, v_uv);
	
	vec3 P = v_position;
	vec3 N = normalize(matNormal.x * v_tangent + matNormal.y * v_bitangent + matNormalY * v_normal);
	vec3 V = normalize(cameraPosition - v_position);
	vec3 cdiff = matAlbedo.xyz * (1.0 - matMask.x);
	vec3 f0 = lerp(asVec3(0.03), matAlbedo, matMask.x);
	float alpha = matMask.w*matMask.w;
	float alpha2 = alpha*alpha;

	result += matAlbedo * matMask.z;

	result += evaluateIBL(P, N, V, cdiff, f0, alpha) * matMask.y;

	int end = int(numLightsF) * SP_POINTLIGHT_DATA_SIZE;
	for (int base = 0; base < end; base += SP_POINTLIGHT_DATA_SIZE) {
		result += evaluatePointLight(P, N, V, cdiff, f0, alpha2, base);
        // result += evaluatePointLightDiffuse(P, N, cdiff, base);
	}

	// HACK AO
	result *= matMask.y;

	result = evaluateVisFog(result, P);

	o_color = tonemap(result);
}

#endif
