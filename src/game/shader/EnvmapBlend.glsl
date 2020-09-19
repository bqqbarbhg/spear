@module EnvmapBlend

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

@glsl_options flip_vert_y

in vec2 pos;

out vec2 v_uv;

void main()
{
	v_uv = pos;
    gl_Position = vec4(pos.x * 2.0 - 1.0, pos.y * -2.0 + 1.0, 0.5, 1.0);
}

@end

@fs fs

uniform sampler2D lighting;
uniform sampler3D blendPrev;
uniform sampler3D envmapPrev;

in vec2 v_uv;

layout(location=0) out vec4 o_blend;
layout(location=1) out vec4 o_envmap;

uniform Pixel
{
    vec4 rayDirs[3];
	vec2 prevShift;
    vec4 uvToLightMad;
    vec4 uvToBlueNoiseMad;
    float depthSlice;
};

vec4 blendAlpha(vec4 prev, vec3 next, float minA, float maxA)
{
    float a = prev.a + 1.0;
    vec3 v = mix(prev.xyz, next.xyz, clamp(1.0 / a, minA, maxA));
    return vec4(v, a);
}

vec3 getLighting(vec2 atlasUv)
{
    float part = floor(atlasUv.x * 6.0);
    vec2 uv = vec2(atlasUv.x * 6.0 - part, atlasUv.y);

    vec2 lightUv = uv * uvToLightMad.xy + uvToLightMad.zw;

    vec3 result = vec3(0.0);
    for (int i = 0; i < 3; i++) {
        vec2 sampleUv = (lightUv + vec2(depthSlice, float(i))) * vec2(1.0/3.0, 1.0/3.0);
        vec3 light = textureLod(lighting, sampleUv, 0.0).xyz;

        // Compensate for PDF
        light *= 4.0 * 3.141 * (1.0/3.0);

        vec3 N = rayDirs[i].xyz;

        float weight = 0.0;
        if (part == 0.0) {
            weight = N.x > 0.0 ? N.x*N.x : 0.0;
        } else if (part == 1.0) {
            weight = N.x < 0.0 ? N.x*N.x : 0.0;
        } else if (part == 2.0) {
            weight = N.y > 0.0 ? N.y*N.y : 0.0;
        } else if (part == 3.0) {
            weight = N.y < 0.0 ? N.y*N.y : 0.0;
        } else if (part == 4.0) {
            weight = N.z > 0.0 ? N.z*N.z : 0.0;
        } else if (part == 5.0) {
            weight = N.z < 0.0 ? N.z*N.z : 0.0;
        }

        result += light * weight;
    }

    return result;
}

void main()
{
    vec2 uv = v_uv;

    float depth = depthSlice * (1.0/3.0) + (0.5/3.0);
    vec3 prevUv = vec3(uv + prevShift, depth);

    vec2 prevCell = floor(prevUv.xy * vec2(6.0, 1.0));
    vec2 cell = floor(uv * vec2(6.0, 1.0));

    vec4 env = textureLod(envmapPrev, prevUv, 0.0);
    vec4 blend = textureLod(blendPrev, prevUv, 0.0);

    if (prevCell != cell) {
        env = blend = vec4(0.0);
    }

    vec3 light = getLighting(uv);

    vec4 newBlend = blendAlpha(blend, light, 0.03, 0.4);
    vec4 newEnvmap = blendAlpha(env, newBlend.xyz, 0.01, 0.2);

    o_blend = newBlend;
    o_envmap = newEnvmap;

#if 0
    float alpha = max((1.0 - prev.a) * 0.1, 0.01);
    alpha = 0.001;

    vec2 noiseUv = uv * uvToBlueNoiseMad.xy + uvToBlueNoiseMad.zw;
    vec4 noise = textureLod(blueNoise, noiseUv, 0.0);
    // alpha *= clamp(noise.x * 4.0 - 2.0, 0.0, 1.0);
    alpha *= clamp(noise.x * 4.0 + 0.99, 0.0, 1.0);

    vec3 light = vec3(0.0);
    vec2 lightUv = uv * uvToLightMad.xy + uvToLightMad.zw;
    if (lightUv == clamp(lightUv, 0.0, 1.0) && lightFactor > 0.0001) {
        light = textureLod(lighting, lightUv, 0.0).xyz * lightFactor;
    }

    vec3 result = mix(prev.xyz, light, alpha);
    o_color = vec4(result, clamp(prev.a + alpha, 0.0, 1.0));
#endif
}

@end

@program EnvmapBlend vs fs


