@module MapTile

@vs vs

uniform Vertex
{
	mat4 worldToClip;
};

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

out vec3 v_normal;
out vec2 v_uv;

void main()
{
    gl_Position = worldToClip * vec4(position, 1.0);
	v_normal = normal;
	v_uv = uv;
}

@end

@fs fs

in vec3 v_normal;
in vec2 v_uv;

out vec4 frag_color;

void main()
{
	float v = dot(normalize(v_normal), normalize(vec3(1.0,1.0,-1.0)));
	frag_color = vec4(vec3(v), 1.0);
}

@end

@program MapTile vs fs

