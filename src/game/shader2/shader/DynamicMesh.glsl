#include "util/Defines.glsl"

layout(location=0) attribute vec3 a_position;
layout(location=1) attribute vec3 a_normal;
layout(location=2) attribute vec4 a_tangent;
layout(location=3) attribute vec2 a_uv;

varying vec3 v_position;
varying vec3 v_normal;
varying vec3 v_tangent;
varying vec3 v_bitangent;
varying vec2 v_uv;

#ifdef SP_VS

uniform DynamicTransform
{
	mat4 modelToWorld;
	mat4 worldToClip;
};

void main()
{
    vec3 p = mul(vec4(a_position, 1.0), modelToWorld).xyz;
    vec3 n = mul(vec4(a_normal, 0.0), modelToWorld).xyz;
    vec3 t = mul(vec4(a_tangent.xyz, 0.0), modelToWorld).xyz;

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

