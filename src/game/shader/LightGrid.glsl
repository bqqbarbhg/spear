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

uniform sampler2D shadowGrid;

#define MAX_LIGHTS 64
#define DATA_PER_LIGHT 2

uniform Pixel
{
	vec3 lightGridOrigin;
	vec3 lightGridScale;
	vec4 lightData[MAX_LIGHTS*DATA_PER_LIGHT];
	float numLightsF;
};

float sampleShadow(vec3 p, int lightBase)
{
	vec4 data2 = lightData[lightBase + 2];
	vec4 data3 = lightData[lightBase + 3];
	vec3 shadowGridOrigin = data2.xyz;
	vec3 shadowGridRcpScale = data3.xyz;
	float shadowGridYSlices = data2.w;
	float shadowGridRcpYSlices = data3.w;

	vec3 rp = clamp((p - shadowGridOrigin) * shadowGridRcpScale, vec3(shadowGridRcpScale), vec3(1.0 - shadowGridRcpScale));
	float y = rp.y * shadowGridYSlices;
	float sliceY = floor(y);
	float dy = y - sliceY;
	float x0 = (rp.x + sliceY) * shadowGridRcpYSlices;
	float x1 = x0 + shadowGridRcpYSlices;
	float z = rp.z;

	float v0 = textureLod(shadowGrid, vec2(x0, z), 0.0).x;
	float v1 = textureLod(shadowGrid, vec2(x1, z), 0.0).x;
	return mix(v0, v1, dy);
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
		float shadow = sampleShadow(p, base);
		vec3 lightPos = data0.xyz;
		vec3 lightCol = data1.xyz;
		vec3 delta = lightPos - p;
		float distSq = dot(delta, delta);
		vec3 l = delta / sqrt(distSq);
		float attenuation = 1.0 / (0.1 + distSq);
		float n_l = clamp(n0 + dot(n, l), 0.0, 1.0);
		sum += n_l * attenuation * lightCol * shadow;
	}

	o_color = vec4(sum, 1.0);
}

@end

@program LightGrid vs fs


