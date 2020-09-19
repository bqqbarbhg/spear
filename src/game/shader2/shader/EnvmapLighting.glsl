
#include "util/Defines.glsl"

layout(location=0) attribute vec2 a_position;

varying vec2 v_uv;

#ifdef SP_VS

uniform EnvmapVertex
{
    float flipX;
    float flipY;
    float pad_2;
    float pad_3;
};

void main()
{
    vec2 pos = a_position;
	v_uv = pos;
    gl_Position = vec4((pos.x * 2.0 - 1.0) * flipX, (pos.y * -2.0 + 1.0) * flipY, 0.5, 1.0);
}

#endif

#ifdef SP_FS

#pragma permutation SP_SHADOWGRID_USE_ARRAY 2
#pragma permutation SP_DEBUG_MODE 2
#pragma permutation SP_DIRECT_ENV_LIGHT 2

#define MAX_LIGHTS 128

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

#ifndef SP_DEBUG_MODE
    #error "Permutation SP_DEBUG_MODE not defined"
#endif

vec3 decodeOctahedralNormal(vec2 f)
{
    f = f * 2.0 - 1.0;
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 n = vec3( f.x, f.y, 1.0 - abs( f.x ) - abs( f.y ) );
    float t = saturate( -n.z );
    n.x += n.x >= 0.0 ? -t : t;
    n.y += n.y >= 0.0 ? -t : t;
    return normalize( n );
}

void main()
{
    // TODO: texelFetch()
    vec2 uv = v_uv;
    vec2 sampleUv = uv * uvMad.xy + uvMad.zw;
    vec4 g0 = textureLod(gbuffer0, sampleUv, 0.0);
    vec4 g1 = textureLod(gbuffer1, sampleUv, 0.0);

    vec3 albedo = srgbToLinear(g0.xyz);
    // vec3 albedo = g0.xyz;
    // vec3 albedo = asVec3(0.5);
    vec3 normal = decodeOctahedralNormal(g1.xy);
    float depth = g0.w + g1.w * (1.0 / 256.0);

    float dist = depthToDistance * depth;
    vec3 cdiff = albedo;

    #if defined(SP_GLSL)
        vec4 clipP = vec4(uv.x * 2.0 - 1.0, uv.y * -2.0 + 1.0, depth * 2.0 - 1.0, 1.0);
    #else
        vec4 clipP = vec4(uv.x * 2.0 - 1.0, uv.y * -2.0 + 1.0, depth, 1.0);
    #endif
    vec4 worldP = mul(clipP, clipToWorld);
    vec3 P = worldP.xyz * (1.0 / worldP.w);
    vec3 N = normal;
    float roughness = g1.z;

    // Sphere position for direct lighting
#if SP_DIRECT_ENV_LIGHT
    #if defined(SP_GLSL)
        vec4 clipSP = vec4(uv.x * 2.0 - 1.0, uv.y * -2.0 + 1.0, -1.0, 1.0);
    #else
        vec4 clipSP = vec4(uv.x * 2.0 - 1.0, uv.y * -2.0 + 1.0, 0.0, 1.0);
    #endif
    vec4 worldSP = mul(clipSP, clipToWorld);
    vec3 SP = worldSP.xyz * (1.0 / worldSP.w);
#endif

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
            roughness = 0.7;
        }
    #endif

	float alpha = roughness*roughness;
	vec3 f0 = asVec3(0.03 * g1.z);
	float alpha2 = alpha*alpha;

    if (depth > 0.0) {
        result += evaluateIBLDiffuse(P, N, cdiff);// * 0.5;

        int end = int(numLightsF) * SP_POINTLIGHT_DATA_SIZE;
        for (int base = 0; base < end; base += SP_POINTLIGHT_DATA_SIZE) {
            result += evaluatePointLight(P, N, V, cdiff, f0, alpha2, base);
            #if SP_DIRECT_ENV_LIGHT
                result += evaluatePointLightDiffuse(SP, rayDir, asVec3(1.0), base);
            #endif
        }
    }

    // result *= 1.0 / (1.0 + dist * dist);

    float len = length(result);
    if (len > 2.0) result *= 2.0 / len;

    o_color = vec4(result, 1.0);
}

#endif

