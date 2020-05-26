@module TestMesh

@vs vs

uniform Transform
{
	mat4 transform;
	mat4 normalTransform;
};

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec4 tangent;
layout(location=3) in vec2 uv;

out vec3 v_position;
out vec3 v_normal;
out vec3 v_tangent;
out vec3 v_bitangent;
out vec2 v_uv;

void main()
{
	v_position = position;
    gl_Position = transform * vec4(position, 1.0);
	v_normal = normal;
	v_tangent = tangent.xyz;
	v_bitangent = tangent.w * cross(v_normal, v_tangent); 
	v_uv = uv;
}

@end

@fs fs

@include Brdf.inc.glsl

in vec3 v_position;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec2 v_uv;

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

uniform sampler2D albedoAtlas;
uniform sampler2D normalAtlas;
uniform sampler2D maskAtlas;

uniform Pixel
{
	float numLightsF;
	vec3 cameraPosition;
	vec4 lightData[MAX_LIGHTS*DATA_PER_LIGHT];
};

vec3 evalLight(vec3 P, vec3 N, vec3 V, vec3 cdiff, vec3 f0, float alpha2, int base)
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

		vec3 H = normalize(L + V);
		float VdotH = clamp(dot(V, H), 0.0, 1.0);
		float NdotL = clamp(dot(N, L), 0.0, 1.0);
		float NdotV = clamp(dot(N, V), 0.0, 1.0);
		float NdotH = clamp(dot(N, H), 0.0, 1.0);

		result = vec3(shadow * attenuation * NdotL) * lightColor * BRDF_specularGGX(cdiff, f0, alpha2, VdotH, NdotL, NdotV, NdotH);
		// result.xy *= 0.001;
		// result.xy += fract(shadowTexCoord.xz * 20.0);
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
	vec3 result = vec3(0.0);
	int end = int(numLightsF) * DATA_PER_LIGHT;

	vec2 matNormal = texture(normalAtlas, v_uv).xy * 2.0 - vec2(1.0);
	float matNormalY = sqrt(clamp(1.0 - dot(matNormal, matNormal), 0.0, 1.0));

	vec3 matAlbedo = texture(albedoAtlas, v_uv).xyz;
	vec4 matMask = texture(maskAtlas, v_uv);
	
	vec3 P = v_position;
	vec3 N = normalize(matNormal.x * v_tangent + matNormal.y * v_bitangent + matNormalY * v_normal);
	vec3 V = normalize(cameraPosition - v_position);
	vec3 cdiff = matAlbedo.xyz * (1.0 - matMask.x);
	vec3 f0 = mix(vec3(0.03), matAlbedo, matMask.x);
	float alpha = matMask.w*matMask.w;
	float alpha2 = alpha*alpha;

	for (int base = 0; base < end; base += DATA_PER_LIGHT) {
		result += evalLight(P, N, V, cdiff, f0, alpha2, base);
	}

	o_color = tonemap(result);
}

@end

@program TestMesh vs fs

