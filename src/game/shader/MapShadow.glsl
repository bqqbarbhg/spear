@module MapShadow

@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

@glsl_options flip_vert_y

uniform Vertex
{
	vec3 cameraPosition;
	mat4 worldToClip;
};

layout(location=0) in vec3 position;

out float v_distance;

void main()
{
    gl_Position = worldToClip * vec4(position, 1.0);
	v_distance = length(position - cameraPosition);
}

@end

@fs fs

in float v_distance;
out float o_color;

void main()
{
	o_color = v_distance;
}

@end

@program MapShadow vs fs


