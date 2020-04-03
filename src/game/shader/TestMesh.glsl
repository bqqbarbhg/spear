@module TestMesh

@vs vs

uniform Transform
{
	mat4 transform;
	mat4 normalTrasnform;
	vec3 color;
	vec4 texScaleOffset;
};

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

out vec3 v_normal;
out vec3 v_color;
out vec2 v_uv;

void main()
{
    gl_Position = transform * vec4(position, 1.0);
	v_normal = normalize((normalTrasnform * vec4(normal, 0.0)).xyz);
	v_color = color;
	v_uv = uv * texScaleOffset.xy + texScaleOffset.zw;
}

@end

@fs fs

in vec3 v_normal;
in vec3 v_color;
in vec2 v_uv;

out vec4 frag_color;

uniform FragUniform
{
	vec2 texMin;
	vec2 texMax;
};

uniform sampler2D albedo;

void main()
{
	float v = dot(normalize(v_normal), normalize(vec3(1.0,1.0,-1.0)));
	if (v_uv.x < texMin.x || v_uv.y < texMin.y || v_uv.x > texMax.x || v_uv.y > texMax.y) discard;
	vec4 tex = texture(albedo, v_uv);
	tex.xyz *= v_color * sqrt(v*0.5+0.5);

	tex.a = (tex.a - 0.3) / 0.4;

    frag_color = tex;
}

@end

@program TestMesh vs fs

