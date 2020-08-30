@module CharacterOutline

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

uniform Vertex
{
    vec2 u_PatternSize;
};

in vec2 pos;

out float v_pattern;

void main()
{
	v_pattern = dot(pos * u_PatternSize, vec2(1.0, 1.0));
    gl_Position = vec4(pos.x * 2.0 - 1.0, pos.y * -2.0 + 1.0, 0.5, 1.0);
}

@end

@fs fs

in float v_pattern;

out vec4 o_color;

void main()
{
	o_color = vec4(0.2);
}

@end

@program CharacterOutline vs fs

