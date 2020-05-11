@module TestSkin

@vs vs

#define MAX_BONES 64

uniform VertexUniform
{
	vec3 color;
	vec4 texScaleOffset;
	mat4 viewProj;
};

uniform Bones
{
	vec4 bones[MAX_BONES * 3];
};

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
#if SOKOL_GLSL
	layout(location=3) in vec4 indices;
#else
	layout(location=3) in uvec4 indices;
#endif
layout(location=4) in vec4 weights;

out vec3 v_normal;
out vec3 v_color;
out vec2 v_uv;

void main()
{
#if SOKOL_GLSL
	ivec4 ix = ivec4(indices * 3.0);
#else
	ivec4 ix = ivec4(indices) * 3;
#endif
	vec4 row0 = bones[ix.x + 0] * weights.x;
	vec4 row1 = bones[ix.x + 1] * weights.x;
	vec4 row2 = bones[ix.x + 2] * weights.x;
	row0 += bones[ix.y + 0] * weights.y;
	row1 += bones[ix.y + 1] * weights.y;
	row2 += bones[ix.y + 2] * weights.y;
	row0 += bones[ix.z + 0] * weights.z;
	row1 += bones[ix.z + 1] * weights.z;
	row2 += bones[ix.z + 2] * weights.z;
	row0 += bones[ix.w + 0] * weights.w;
	row1 += bones[ix.w + 1] * weights.w;
	row2 += bones[ix.w + 2] * weights.w;

	vec3 p = vec3(
		dot(row0, vec4(position, 1.0)),
		dot(row1, vec4(position, 1.0)),
		dot(row2, vec4(position, 1.0)));

	vec3 n = vec3(
		dot(row0, vec4(normal, 0.0)),
		dot(row1, vec4(normal, 0.0)),
		dot(row2, vec4(normal, 0.0)));

    gl_Position = viewProj * vec4(p, 1.0);
	v_normal = normalize(n);
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

@program TestSkin vs fs

