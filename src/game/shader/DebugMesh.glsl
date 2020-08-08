@module DebugMesh

@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

uniform Vertex
{
	mat4 worldToClip;
};

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;

out vec3 v_normal;

void main()
{
    gl_Position = worldToClip * vec4(position, 1.0);
	v_normal = normal;
}

@end

@fs fs

in vec3 v_normal;
out vec4 o_color;

void main()
{
    float v = dot(v_normal, normalize(vec3(1.0))) * 0.5 + 0.5;
	o_color = vec4(1.0, v, v, 1.0);
}

@end

@program DebugMesh vs fs



