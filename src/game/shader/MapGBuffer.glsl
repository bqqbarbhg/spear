@module MapGBuffer

@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

uniform Vertex
{
	mat4 worldToClip;
};

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 tangent;
layout(location=3) in vec2 uv;
layout(location=4) in vec4 tint;

out vec2 v_uv;
out vec3 v_normal;
flat out vec3 v_tint;

void main()
{
    vec4 pos = worldToClip * vec4(position, 1.0);
    gl_Position = pos;
    v_uv = uv;
    v_normal = normal;
    v_tint = tint.xyz;
}

@end

@fs fs

in vec2 v_uv;
in vec3 v_normal;
flat in vec3 v_tint;

uniform sampler2D albedoAtlas;

layout(location=0) out vec4 o_gbuffer0;
layout(location=1) out vec4 o_gbuffer1;

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

void main()
{
    vec3 albedo = textureLod(albedoAtlas, v_uv, 5.0).xyz * v_tint;
    float depth = gl_FragCoord.z;
    float depth1 = fract(depth * 256.0);
    depth -= depth1 * (1.0 / 256.0);

    vec3 normal = v_normal;
    if (!gl_FrontFacing) {
        albedo = vec3(0.0);
        normal = -normal;
    }

	o_gbuffer0 = vec4(linearToSrgb(albedo), depth);
    o_gbuffer1 = vec4(v_normal * 0.5 + 0.5, depth1);
}

@end

@program MapGBuffer vs fs
