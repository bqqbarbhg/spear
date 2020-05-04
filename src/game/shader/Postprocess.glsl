@module Postprocess

@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

@glsl_options flip_vert_y

in vec2 pos;

out vec2 v_uv;

void main()
{
	v_uv = pos;
    gl_Position = vec4(pos.x * 2.0 - 1.0, pos.y * -2.0 + 1.0, 0.5, 1.0);
}

@end

@fs fs

uniform sampler2D mainImage;

in vec2 v_uv;

out vec4 o_color;

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
	return v / (1.0 + v);
}

void main()
{
	vec3 color = texture(mainImage, v_uv).xyz;
	color = tonemap(color * 4.0);
	color = linearToSrgb(color);
	float luma = dot(color.rgb, vec3(0.299, 0.587, 0.114));
	o_color = vec4(color, luma);
}

@end

@program Postprocess vs fs

