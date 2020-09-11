
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

#define MAX_LIGHTS 32

uniform EnvmapPixel
{
    mat4 clipToWorld;
	float numLightsF;
    float depthToDistance;
    vec4 blueNoiseMad;
	vec4 pointLightData[MAX_LIGHTS*SP_POINTLIGHT_DATA_SIZE];
};

#include "light/PointLight.glsl"
#include "util/Srgb.glsl"

out vec4 o_color;

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;

void main()
{
    // TODO: texelFetch()
    vec2 uv = v_uv;
    vec4 g0 = textureLod(gbuffer0, uv, 0);
    vec4 g1 = textureLod(gbuffer1, uv, 0);

    vec2 blueNoiseUv = uv * blueNoiseMad.xy + blueNoiseMad.zw;

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

    vec3 result = asVec3(0.0);

	int end = int(numLightsF) * SP_POINTLIGHT_DATA_SIZE;
	for (int base = 0; base < end; base += SP_POINTLIGHT_DATA_SIZE) {
		result += evaluatePointLightDiffuse(P, N, cdiff, base);
	}

    // result *= 1.0 / (1.0 + dist * dist);

    float len = length(result);
    if (len > 1.0) result /= len;

    result *= 10.0 * 3.141;

    o_color = vec4(result, 1.0);
}

#endif

