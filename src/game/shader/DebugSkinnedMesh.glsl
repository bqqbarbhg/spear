@module DebugSkinnedMesh

@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

#define SP_MAX_BONES 64

layout(location=0) in vec3 a_position;
layout(location=1) in vec2 a_uv;
layout(location=2) in vec3 a_normal;
layout(location=3) in vec4 a_tangent;
layout(location=4) in uvec4 a_indices;
layout(location=5) in vec4 a_weights;

out vec3 v_normal;

uniform SkinTransform
{
	mat4 worldToClip;
};

uniform Bones
{
	vec4 bones[SP_MAX_BONES * 3];
};

void main()
{
#if SOKOL_GLSL
	ivec4 ix = ivec4(a_indices * 3.0);
#else
	ivec4 ix = ivec4(a_indices) * 3;
#endif
    vec4 weights = a_weights;
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

    vec4 pos = vec4(a_position, 1.0);
	vec3 p = vec3(
		dot(row0, pos),
		dot(row1, pos),
		dot(row2, pos));

    vec4 nrm = vec4(a_normal, 0.0);
	vec3 n = vec3(
		dot(row0, nrm),
		dot(row1, nrm),
		dot(row2, nrm));

    vec4 tng = vec4(a_tangent.xyz, 0.0);
	vec3 t = vec3(
		dot(row0, tng),
		dot(row1, tng),
		dot(row2, tng));

    gl_Position = worldToClip * vec4(p, 1.0);
	v_normal = normalize(n);

    v_normal.x += (a_uv.x + a_tangent.x) * 0.00001;
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

@program DebugSkinnedMesh vs fs
