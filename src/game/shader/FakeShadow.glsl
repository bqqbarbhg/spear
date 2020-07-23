@module FakeShadow

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

layout(location=0) in vec3 a_Position;
layout(location=1) in vec3 a_UvAlpha;

uniform Vertex
{
	mat4 worldToClip;
};

out vec2 v_Uv;
flat out float v_Alpha;

void main()
{
    gl_Position = worldToClip * vec4(a_Position, 1.0);
    v_Uv = a_UvAlpha.xy;
    v_Alpha = a_UvAlpha.z;
}

@end

@fs fs

in vec2 v_Uv;
flat in float v_Alpha;

out vec4 o_Color;

void main()
{
    float t = clamp(length(v_Uv), 0.0, 1.0);
    t = t * t * (3.0 - 2.0 * t);
    float a = (1.0 - t) * v_Alpha;
    o_Color = vec4(0.0, 0.0, 0.0, a);
}

@end

@program FakeShadow vs fs

