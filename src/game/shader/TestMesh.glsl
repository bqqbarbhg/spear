@module TestMesh

@vs vs

uniform Transform
{
	mat4 transform;
	mat4 normalTransform;
};

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 tangent;
layout(location=3) in vec2 uv;

out vec3 v_position;
out vec3 v_normal;

void main()
{
	v_position = position;
    gl_Position = transform * vec4(position, 1.0);
	v_normal = normalize((normalTransform * vec4(normal, 0.0)).xyz);
}

@end

@fs fs

in vec3 v_position;
in vec3 v_normal;

out vec4 o_color;

#define USE_ARRAY 1

#define MAX_LIGHTS 16
#define DATA_PER_LIGHT 4
// vec3 origin; float radius;
// vec3 color; float ?
// vec3 shadowMul; float ?
// vec3 shadowBias; float ?

#if USE_ARRAY
	uniform sampler2DArray shadowGrid;
#else
	uniform sampler3D shadowGrid;
#endif

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

#if USE_ARRAY
		float ly = shadowTexCoord.y * 8.0;
		float l0 = clamp(floor(ly), 0.0, 7.0);
		float l1 = min(l0 + 1.0, 7.0);
		float t = ly - l0;
		float s0 = textureLod(shadowGrid, vec3(shadowTexCoord.xz, l0), 0.0).x;
		float s1 = textureLod(shadowGrid, vec3(shadowTexCoord.xz, l1), 0.0).x;
		float shadow = mix(s0, s1, t);
#else
		float shadow = textureLod(shadowGrid, shadowTexCoord.xzy, 0.0).x;
#endif

		float attenuation = 1.0 / (0.1 + distSq) - 1.0 / (0.1 + radiusSq);
		result = dot(N, L) * vec3(shadow) * attenuation * lightColor;

	} else {
		result = vec3(0.0);
	}

	return result;
}

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

vec4 tonemap(vec3 v)
{
	vec3 x = v / (1.0 + v);
	x = linearToSrgb(x);
	return vec4(x, dot(x, vec3(0.299, 0.587, 0.114)));
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
	o_color = tonemap(result);
}

@end

@program TestMesh vs fs

