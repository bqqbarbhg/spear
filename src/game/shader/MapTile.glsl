@module MapTile

@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

uniform Vertex
{
	mat4 worldToClip;
};

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_uv;

void main()
{
    gl_Position = worldToClip * vec4(position, 1.0);
	v_position = position;
	v_normal = normal;
	v_uv = uv;
}

@end

@fs fs

in vec3 v_position;
in vec3 v_normal;
in vec2 v_uv;

out vec4 frag_color;

uniform sampler2D lightGrid;

uniform Pixel
{
	vec3 lightGridOrigin;
	vec3 lightGridRcpScale;
	float lightGridYSlices;
	float lightGridRcpYSlices;
	vec3 cameraPosition;
};

vec3 sampleIrradianceImp(float x0, float x1, float y, float dy)
{
	vec3 v0 = texture(lightGrid, vec2(x0, y)).xyz;
	vec3 v1 = texture(lightGrid, vec2(x1, y)).xyz;
	return mix(v0, v1, dy);
}

vec3 sampleIrradiance(vec3 p, vec3 n)
{
	vec3 rp = clamp((p - lightGridOrigin) * lightGridRcpScale, vec3(lightGridRcpScale), vec3(1.0 - lightGridRcpScale));
	float y = rp.y * lightGridYSlices;
	float sliceY = floor(y);
	float dy = y - sliceY;
	float x0 = (rp.x + sliceY) * lightGridRcpYSlices;
	float x1 = x0 + lightGridRcpYSlices;
	float z = rp.z * (1.0 / 7.0);

	vec3 result;
	result  = n.x*n.x*sampleIrradianceImp(x0, x1, z + (n.x>0 ? 0.0/7.0 : 1.0/7.0), dy);
	result += n.y*n.y*sampleIrradianceImp(x0, x1, z + (n.y>0 ? 2.0/7.0 : 3.0/7.0), dy);
	result += n.z*n.z*sampleIrradianceImp(x0, x1, z + (n.z>0 ? 4.0/7.0 : 5.0/7.0), dy);
	return result;
}

vec4 sampleVolumeIrradiance(vec3 p)
{
	vec3 rp = clamp((p - lightGridOrigin) * lightGridRcpScale, vec3(lightGridRcpScale), vec3(1.0 - lightGridRcpScale));
	float y = rp.y * lightGridYSlices;
	float sliceY = floor(y);
	float dy = y - sliceY;
	float x0 = (rp.x + sliceY) * lightGridRcpYSlices;
	float x1 = x0 + lightGridRcpYSlices;
	float z = rp.z * (1.0 / 7.0) + (6.0 / 7.0);

	vec4 v0 = textureLod(lightGrid, vec2(x0, z), 0.0);
	vec4 v1 = textureLod(lightGrid, vec2(x1, z), 0.0);
	return mix(v0, v1, dy);
}

void main()
{
	vec3 p = v_position;
	vec3 n = normalize(v_normal);
	vec3 result = sampleIrradiance(v_position, n);

#if 0
	vec3 dir = cameraPosition - p;
	float dist = length(dir);
	dir /= dist;
	float t = 0.0;
	const float tStep = 0.5;
	for (int i = 0; i < 64; i++, t += tStep) {
		if (t >= dist) break;
		vec3 vp = p + dir * t;
		float density = clamp(1.0 - vp.y * 0.05, 0.0, 1.0) * 0.5;
		density *= tStep;
		vec4 vol = sampleVolumeIrradiance(vp);
		result *= 1.0 - density*vol.w;
		result += vol.xyz * density;
	}
#endif

	frag_color = vec4(sqrt(result), 1.0);
}

@end

@program MapTile vs fs

