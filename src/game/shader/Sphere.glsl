@module Sphere

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

layout(location=0) in vec3 a_position;
layout(location=1) in vec4 a_color;
layout(location=2) in vec4 a_row0;
layout(location=3) in vec4 a_row1;
layout(location=4) in vec4 a_row2;

flat out vec3 v_color;

uniform Vertex
{
	mat4 worldToClip;
};

void main()
{
	vec3 p = vec3(
		dot(a_row0, vec4(a_position, 1.0)),
		dot(a_row1, vec4(a_position, 1.0)),
		dot(a_row2, vec4(a_position, 1.0)));
    gl_Position = worldToClip * vec4(p, 1.0);
	v_color = a_color.xyz;
}

@end

@fs fs

flat in vec3 v_color;

out vec4 o_color;

void main()
{
	o_color = vec4(v_color, 1.0);
}

@end

@program Sphere vs fs

