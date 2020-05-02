@module LightGrid

@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

@glsl_options flip_vert_y

in vec2 pos;

out vec2 v_uv;

uniform Vertex
{
	float lightGridYSlices;
};

void main()
{
	v_uv = pos * vec2(lightGridYSlices, 7.0);
    gl_Position = vec4(pos.x * 2.0 - 1.0, pos.y * -2.0 + 1.0, 0.5, 1.0);
}

@end

@fs fs

in vec2 v_uv;

out vec4 o_color;

uniform sampler2D shadowAtlas;

#define MAX_LIGHTS 64
#define DATA_PER_LIGHT 3

uniform Pixel
{
	vec3 lightGridOrigin;
	vec3 lightGridScale;
	vec4 lightData[MAX_LIGHTS*DATA_PER_LIGHT];
	float numLightsF;
};

float sampleShadowImp(vec3 delta, vec4 uvMad, float dist)
{
	vec3 absDelta = abs(delta);
	float ma, faceIndex;
	vec2 uv;
	if (absDelta.z >= absDelta.x && absDelta.z >= absDelta.y) {
		faceIndex = 5.0 - 0.5*sign(delta.z);
		ma = 0.5 / absDelta.z;
		uv = vec2(sign(delta.z) * -delta.x, -delta.y);
	} else if (absDelta.y >= absDelta.x) {
		faceIndex = 3.0 - 0.5*sign(delta.y);
		ma = 0.5 / absDelta.y;
		uv = vec2(delta.x, sign(delta.y) * -delta.z);
	} else {
		faceIndex = 1.0 - 0.5*sign(delta.x);
		ma = 0.5 / absDelta.x;
		uv = vec2(sign(delta.x) * delta.z, -delta.y);
	}
	uv.x = uv.x * ma;
	uv.y = uv.y * ma;
	uv = clamp(uv, vec2(-0.499), vec2(0.499));
	uv.x += faceIndex;
	uv.y += 0.5;
	float ref = textureLod(shadowAtlas, uv * uvMad.xy + uvMad.zw, 0.0).x;
	return ref < dist ? 0.0 : 1.0;
}

float sampleShadow(vec3 delta, float dist, int lightBase)
{
	vec4 uvMad = lightData[lightBase + 2];
	float sum = 0.0, total = 1.0;
	delta = -delta;
	sum = sampleShadowImp(delta, uvMad, dist - 0.3);
#if 0
	float pcfRadius = 0.25;
	float pcfStep = 0.25;
	for (float dz = -pcfRadius; dz <= pcfRadius + 0.0001; dz += pcfStep)
	for (float dy = -pcfRadius; dy <= pcfRadius + 0.0001; dy += pcfStep)
	for (float dx = -pcfRadius; dx <= pcfRadius + 0.0001; dx += pcfStep)
	{
		float bias = abs(dx) + abs(dy) + abs(dz);
		sum += sampleShadowImp(delta + vec3(dx, dy, dz), uvMad, dist - 0.3);
		total += 1.0;
	}
#endif
	float d = 0.25;
	sum += sampleShadowImp(delta + vec3(+d, 0.0, 0.0), uvMad, dist - 0.3);
	sum += sampleShadowImp(delta + vec3(-d, 0.0, 0.0), uvMad, dist - 0.3);
	sum += sampleShadowImp(delta + vec3(0.0, +d, 0.0), uvMad, dist - 0.3);
	sum += sampleShadowImp(delta + vec3(0.0, -d, 0.0), uvMad, dist - 0.3);
	sum += sampleShadowImp(delta + vec3(0.0, 0.0, +d), uvMad, dist - 0.3);
	sum += sampleShadowImp(delta + vec3(0.0, 0.0, -d), uvMad, dist - 0.3);
	return sum / 7.0;
}

void main()
{
	float y = floor(v_uv.x);
	float dir = floor(v_uv.y);
	float x = v_uv.x - y;
	float z = v_uv.y - dir;

	vec3 n;
	float s0 = step(2.0, dir);
	float s1 = step(4.0, dir);
	float s2 = step(6.0, dir);
	dir *= 2.0;
	n.x = (1.0 - s0) * (1.0 - dir);
	n.y = (s0 - s1) * (5.0 - dir);
	n.z = (s1 - s2) * (9.0 - dir);
	float n0 = s2;

	vec3 p = vec3(x, y, z) * lightGridScale + lightGridOrigin;

	vec3 sum = vec3(0.0);

	int numLights = int(numLightsF);
	for (int i = 0; i < MAX_LIGHTS; i++) {
		if (i >= numLights) break;

		int base = i * DATA_PER_LIGHT;
		vec4 data0 = lightData[base + 0];
		vec4 data1 = lightData[base + 1];
		vec3 lightPos = data0.xyz;
		vec3 lightCol = data1.xyz;
		vec3 delta = lightPos - p;
		float distSq = dot(delta, delta);
		float dist = sqrt(distSq);
		vec3 l = delta / dist;
		float shadow = sampleShadow(delta, dist, base);
		float attenuation = 1.0 / (0.1 + distSq);
		float n_l = clamp(n0 + dot(n, l), 0.0, 1.0);
		sum += n_l * attenuation * lightCol * shadow;
	}

	o_color = vec4(sum, 1.0);
}

@end

@program LightGrid vs fs


