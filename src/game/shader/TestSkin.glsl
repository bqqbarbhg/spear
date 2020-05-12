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

out vec3 v_position;
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
	v_position = p;
	v_normal = normalize(n);
	v_color = color;
	v_uv = uv * texScaleOffset.xy + texScaleOffset.zw;
}

@end

@fs fs

in vec3 v_position;
in vec3 v_normal;
in vec3 v_color;
in vec2 v_uv;

out vec4 o_color;

uniform FragUniform
{
	vec2 texMin;
	vec2 texMax;
};

#define MAX_LIGHTS 16
#define DATA_PER_LIGHT 4
// vec3 origin; float radius;
// vec3 color; float ?
// vec3 shadowMul; float ?
// vec3 shadowBias; float ?

uniform sampler3D shadowGrid;

uniform Pixel
{
	float numLightsF;
	vec4 lightData[MAX_LIGHTS*DATA_PER_LIGHT];
};

vec3 evalLight(vec3 P, vec3 N, int base)
{
	vec4 data0 = lightData[base + 0];
	vec4 data1 = lightData[base + 1];
	vec4 data2 = lightData[base + 2];
	vec4 data3 = lightData[base + 3];

	vec3 lightOrigin = data0.xyz;
	float lightRadius = data0.w;

	vec3 delta = lightOrigin - P;
	float distSq = dot(delta, delta);
	vec3 L = delta / sqrt(distSq);

	vec3 result;
	float radiusSq = lightRadius*lightRadius;
	if (distSq < radiusSq) {

		vec3 lightColor = data1.xyz;
		vec3 shadowMul = data2.xyz;
		vec3 shadowBias = data3.xyz;
		vec3 shadowTexCoord = delta * shadowMul + shadowBias;
		float shadow = textureLod(shadowGrid, shadowTexCoord.xzy, 0.0).x;

		float attenuation = 1.0 / (0.1 + distSq) - 1.0 / (0.1 + radiusSq);
		result = dot(N, L) * vec3(shadow) * attenuation * lightColor;

	} else {
		result = vec3(0.0);
	}

	return result;
}

uniform sampler2D albedo;

float linearToSrgb(float x)
{
	x = clamp(x, 0.0, 1.0);
	if (x <= 0.00031308)
		return 12.92 * x;
	else
		return 1.055*pow(x,(1.0 / 2.4) ) - 0.055;
}

vec3 linearToSrgb(vec3 v)
{
	return vec3(linearToSrgb(v.x), linearToSrgb(v.y), linearToSrgb(v.z));
}

vec3 tonemap(vec3 v)
{
	vec3 x = v / (1.0 + v);
	return linearToSrgb(x);
}

void main()
{
	vec3 P = v_position;
	vec3 N = normalize(v_normal);
	vec3 result = vec3(0.0);
	int end = int(numLightsF) * DATA_PER_LIGHT;
	for (int base = 0; base < end; base += DATA_PER_LIGHT) {
		result += evalLight(P, N, base);
	}

	if (v_uv.x < texMin.x || v_uv.y < texMin.y || v_uv.x > texMax.x || v_uv.y > texMax.y) discard;
	vec3 tex = texture(albedo, v_uv).xyz;
	tex *= v_color;

	o_color = vec4(tex * tonemap(result), 1.0);
}

@end

@program TestSkin vs fs

