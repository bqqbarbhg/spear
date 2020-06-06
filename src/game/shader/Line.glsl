@module Line

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

uniform Vertex
{
	mat4 worldToClip;
};

layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_color;

flat out vec3 v_color;

void main()
{
    gl_Position = worldToClip * vec4(a_position, 1.0);
	v_color = a_color;
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

@program Line vs fs

