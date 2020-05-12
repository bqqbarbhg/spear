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
	vec4 lightData[MAX_LIGHTS];
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
	o_color = vec4(tonemap(result), 1.0);
}

@end

@program TestMesh vs fs

