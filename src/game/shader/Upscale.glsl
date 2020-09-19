@module Upscale

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

@glsl_options flip_vert_y

in vec2 pos;

out vec2 v_uv;

uniform Vertex
{
	vec4 uvMad;
};

void main()
{
	v_uv = pos * uvMad.xy + uvMad.zw;
    gl_Position = vec4(pos.x * 2.0 - 1.0, pos.y * -2.0 + 1.0, 0.5, 1.0);
}

@end

@fs fs

in vec2 v_uv;

out vec4 o_color;

uniform sampler2D tonemapImage;

uniform Pixel
{
	vec2 texSize;
	vec2 rcpTexSize;
};

void main()
{
#if 0
	vec2 iTc = v_uv * texSize;
	vec2 tc = floor( iTc - 0.5 ) + 0.5;

	vec2 f = iTc - tc;
	vec2 f2 = f * f;
	vec2 f3 = f2 * f;

    vec2 w0 = (-1.0/6.0)*f3 + (3.0/6.0)*f2 + (-3.0/6.0)*f + (1.0/6.0);
    vec2 w1 = (3.0/6.0)*f3 - f2 + (4.0/6.0);
    vec2 w2 = (-3.0/6.0)*f3 + (3.0/6.0)*f2 + (3.0/6.0)*f + (1.0/6.0);
    vec2 w3 = (1.0/6.0) * f3;

	vec2 s0 = w0 + w1;
	vec2 s1 = w2 + w3;

    vec2 f0 = w1 / (w0 + w1);
    vec2 f1 = w3 / (w2 + w3);
 
    vec2 t0 = (tc - vec2(1.0) + f0) * rcpTexSize;
    vec2 t1 = (tc + vec2(1.0) + f1) * rcpTexSize;

	vec3 color
		= texture(tonemapImage, vec2(t0.x, t0.y)).xyz * s0.x * s0.y
		+ texture(tonemapImage, vec2(t1.x, t0.y)).xyz * s1.x * s0.y
		+ texture(tonemapImage, vec2(t0.x, t1.y)).xyz * s0.x * s1.y
		+ texture(tonemapImage, vec2(t1.x, t1.y)).xyz * s1.x * s1.y;

#elif 1

	// We're going to sample a a 4x4 grid of texels surrounding the target UV coordinate. We'll do this by rounding
	// down the sample location to get the exact center of our "starting" texel. The starting texel will be at
	// location [1, 1] in the grid, where [0, 0] is the top left corner.
	vec2 samplePos = v_uv * texSize;
	vec2 texPos1 = floor(samplePos - 0.5f) + 0.5f;

	// Compute the fractional offset from our starting texel to our original sample location, which we'll
	// feed into the Catmull-Rom spline function to get our filter weights.
	vec2 f = samplePos - texPos1;

	// Compute the Catmull-Rom weights using the fractional offset that we calculated earlier.
	// These equations are pre-expanded based on our knowledge of where the texels will be located,
	// which lets us avoid having to evaluate a piece-wise function.
	vec2 w0 = f * (-0.5f + f * (1.0f - 0.5f * f));
	vec2 w1 = 1.0f + f * f * (-2.5f + 1.5f * f);
	vec2 w2 = f * (0.5f + f * (2.0f - 1.5f * f));
	vec2 w3 = f * f * (-0.5f + 0.5f * f);

	// Work out weighting factors and sampling offsets that will let us use bilinear filtering to
	// simultaneously evaluate the middle 2 samples from the 4x4 grid.
	vec2 w12 = w1 + w2;
	vec2 offset12 = w2 / (w1 + w2);

	// Compute the final UV coordinates we'll use for sampling the texture
	vec2 texPos0 = texPos1 - 1;
	vec2 texPos3 = texPos1 + 2;
	vec2 texPos12 = texPos1 + offset12;

	texPos0 *= rcpTexSize;
	texPos3 *= rcpTexSize;
	texPos12 *= rcpTexSize;

	vec3 color;
	color  = texture(tonemapImage, vec2(texPos0.x, texPos0.y)).xyz * w0.x * w0.y;
	color += texture(tonemapImage, vec2(texPos12.x, texPos0.y)).xyz * w12.x * w0.y;
	color += texture(tonemapImage, vec2(texPos3.x, texPos0.y)).xyz * w3.x * w0.y;

	color += texture(tonemapImage, vec2(texPos0.x, texPos12.y)).xyz * w0.x * w12.y;
	color += texture(tonemapImage, vec2(texPos12.x, texPos12.y)).xyz * w12.x * w12.y;
	color += texture(tonemapImage, vec2(texPos3.x, texPos12.y)).xyz * w3.x * w12.y;

	color += texture(tonemapImage, vec2(texPos0.x, texPos3.y)).xyz * w0.x * w3.y;
	color += texture(tonemapImage, vec2(texPos12.x, texPos3.y)).xyz * w12.x * w3.y;
	color += texture(tonemapImage, vec2(texPos3.x, texPos3.y)).xyz * w3.x * w3.y;
#else

	vec3 color = texture(tonemapImage, v_uv).xyz;

#endif

	o_color = vec4(color, 1.0);
}

@end

@fs fsFast

in vec2 v_uv;

out vec4 o_color;

uniform sampler2D tonemapImage;

void main()
{
	vec3 color = texture(tonemapImage, v_uv).xyz;
	o_color = vec4(color, 1.0);
}

@end

@program Upscale vs fs
@program UpscaleFast vs fsFast

