
#include "util/Defines.glsl"

layout(location=0) attribute vec2 a_position;

varying vec2 v_uv;

#ifdef SP_VS

uniform EnvmapVertex
{
    float flipY;
};

void main()
{
    vec2 pos = a_position;
	v_uv = pos;
    gl_Position = vec4(pos.x * 2.0 - 1.0, (pos.y * -2.0 + 1.0) * flipY, 0.5, 1.0);
}

#endif

#ifdef SP_FS

#pragma permutation SP_SHADOWGRID_USE_ARRAY 2
#pragma permutation SP_DEBUG_MODE 2

#define MAX_LIGHTS 32

uniform EnvmapPixel
{
    mat4 clipToWorld;
	float numLightsF;
    float depthToDistance;
    vec4 uvMad;
    vec3 rayDir;
    vec4 diffuseEnvmapMad;
	vec4 pointLightData[MAX_LIGHTS*SP_POINTLIGHT_DATA_SIZE];
};

#define IBL_NO_SPECULAR

#include "light/PointLight.glsl"
#include "light/IBL.glsl"
#include "util/Srgb.glsl"

out vec4 o_color;

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;

#ifndef SP_DEBUG_MODE
    #error "Permutation SP_DEBUG_MODE not defined"
#endif

void main()
{
    // TODO: texelFetch()
    vec2 uv = v_uv;
    vec2 sampleUv = uv * uvMad.xy + uvMad.zw;
    vec4 g0 = textureLod(gbuffer0, sampleUv, 0);
    vec4 g1 = textureLod(gbuffer1, sampleUv, 0);
    vec4 g2 = textureLod(gbuffer2, sampleUv, 0);

    vec3 albedo = srgbToLinear(g0.xyz);
    // vec3 albedo = g0.xyz;
    // vec3 albedo = asVec3(0.5);
    vec3 normal = normalize(g1.xyz * 2.0 - 1.0);
    float depth = g0.w + g1.w * (1.0 / 256.0);

    float dist = depthToDistance * depth;
    vec3 cdiff = albedo;

    vec4 clipP = vec4(uv.x * 2.0 - 1.0, uv.y * -2.0 + 1.0, depth, 1.0);
    vec4 worldP = mul(clipP, clipToWorld);
    vec3 P = worldP.xyz * (1.0 / worldP.w);
    vec3 N = normal;
	float alpha = g2.x*g2.x;
	vec3 f0 = asVec3(0.03 * g2.x);
	float alpha2 = alpha*alpha;

    vec3 result = asVec3(0.0);

    #if SP_DEBUG_MODE
        vec3 V = normalize(rayDir - P);
    #else
        vec3 V = -rayDir;

        // HACK CEILING
        if (depth == 0.0 && rayDir.y > 0.0) {
            P += rayDir * ((3.7 - P.y) / rayDir.y);
            N = vec3(0.0, -1.0, 0.0);
            cdiff = asVec3(0.15);
            depth = 1.0;
        }
    #endif

    if (depth > 0.0) {
        result += evaluateIBLDiffuse(P, N, cdiff) * 0.5;

        int end = int(numLightsF) * SP_POINTLIGHT_DATA_SIZE;
        for (int base = 0; base < end; base += SP_POINTLIGHT_DATA_SIZE) {
            result += evaluatePointLight(P, N, V, cdiff, f0, alpha2, base);
        }
    }

    // result *= 1.0 / (1.0 + dist * dist);

    float len = length(result);
    if (len > 2.0) result *= 2.0 / len;

    o_color = vec4(result, 1.0);
}

#endif

