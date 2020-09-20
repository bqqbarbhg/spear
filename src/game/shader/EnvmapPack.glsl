@module EnvmapPack

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

@fs fs1

in vec2 v_uv;

uniform sampler3D envmap;

layout(location=0) out vec4 o_layer0;

void main()
{
    vec2 uv = v_uv;

    vec3 l0 = textureLod(envmap, vec3(uv, 0.5), 0.0).xyz;
    o_layer0 = vec4(l0, 1.0);
}

@end

@fs fs2

in vec2 v_uv;

uniform sampler3D envmap;

layout(location=0) out vec4 o_layer0;
layout(location=1) out vec4 o_layer1;

void main()
{
    vec2 uv = v_uv;

    vec3 l0 = textureLod(envmap, vec3(uv, 0.25), 0.0).xyz;
    vec3 l1 = textureLod(envmap, vec3(uv, 0.75), 0.0).xyz;
    o_layer0 = vec4(l0, 1.0);
    o_layer1 = vec4(l1, 1.0);
}

@end

@fs fs3

in vec2 v_uv;

uniform sampler3D envmap;

layout(location=0) out vec4 o_layer0;
layout(location=1) out vec4 o_layer1;
layout(location=2) out vec4 o_layer2;

void main()
{
    vec2 uv = v_uv;

    vec3 l0 = textureLod(envmap, vec3(uv, (0.5/3.0)), 0.0).xyz;
    vec3 l1 = textureLod(envmap, vec3(uv, (1.5/3.0)), 0.0).xyz;
    vec3 l2 = textureLod(envmap, vec3(uv, (2.5/3.0)), 0.0).xyz;
    o_layer0 = vec4(l0, 1.0);
    o_layer1 = vec4(l1, 1.0);
    o_layer2 = vec4(l2, 1.0);
}

@end


@program EnvmapPack1 vs fs1
@program EnvmapPack2 vs fs2
@program EnvmapPack3 vs fs3
