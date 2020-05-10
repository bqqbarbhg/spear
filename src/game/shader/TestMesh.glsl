@module TestMesh

@vs vs

uniform Transform
{
	mat4 transform;
	mat4 normalTransform;
};

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

out vec3 v_normal;

void main()
{
    gl_Position = transform * vec4(position, 1.0);
	v_normal = normalize((normalTransform * vec4(normal, 0.0)).xyz);
}

@end

@fs fs

in vec3 v_normal;

out vec4 frag_color;

void main()
{
	float v = dot(normalize(v_normal), normalize(vec3(1.0,1.0,-1.0)));
	v = v * 0.5 + 0.5;
	v = max(v, 0.0);
	v *= 0.2;
    frag_color = vec4(vec3(v), 1.0);
}

@end

@program TestMesh vs fs

