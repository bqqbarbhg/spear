@module EnvmapBlend

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

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
uniform sampler2D blueNoise;
uniform sampler2DArray envmapPrev;

in vec2 v_uv;

out vec4 o_color;

uniform Pixel
{
    float lightFactor;
    float prevLevel;
	vec2 prevShift;
    vec4 uvToLightMad;
    vec4 uvToBlueNoiseMad;
};

void main()
{
    vec2 uv = v_uv;
    vec3 prevUv = vec3(uv + prevShift, prevLevel);
    vec4 prev = textureLod(envmapPrev, prevUv, 0.0);

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
}

@end

@program EnvmapBlend vs fs


