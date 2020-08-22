@module Billboard

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

layout(location=0) in vec3 a_Position;
layout(location=1) in vec2 a_Uv;
layout(location=2) in vec4 a_Color;

uniform Vertex
{
	mat4 worldToClip;
};

out vec2 v_Uv;
flat out vec4 v_Color;

void main()
{
    gl_Position = worldToClip * vec4(a_Position, 1.0);
    v_Uv = a_Uv.xy;
    v_Color = a_Color;
}

@end

@fs fs

uniform sampler2D u_Atlas;

in vec2 v_Uv;
flat in vec4 v_Color;

out vec4 o_Color;

void main()
{
    vec4 tex = texture(u_Atlas, v_Uv);
    o_Color = tex * v_Color;
}

@end

@program Billboard vs fs


