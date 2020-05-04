@module Fxaa

@ctype vec2 sf::Vec2
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

in vec2 v_uv;

out vec4 o_color;

uniform sampler2D tonemapImage;

uniform Pixel
{
	vec2 rcpTexSize;
};

#define FxaaInt2 ivec2
#define FxaaFloat2 vec2
#define FxaaTexLod0(t, p) textureLod(t, p, 0.0)
#define FxaaTexOff(t, p, o, r) textureLod(t, p + vec2(o)*rcpFrame, 0.0)

vec3 fxaa(sampler2D tex, vec2 uv, vec2 rcpFrame)
{   
/*---------------------------------------------------------*/
	#define FXAA_SUBPIX_SHIFT (1.0/4.0)
    #define FXAA_REDUCE_MIN   (1.0/128.0)
	#define FXAA_REDUCE_MUL   (1.0/8.0)
	#define FXAA_SPAN_MAX     8.0
/*---------------------------------------------------------*/
	vec2 offUv = uv - rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT);
    float lumaNW = FxaaTexLod0(tex, offUv).a;
    float lumaNE = FxaaTexOff(tex, offUv, FxaaInt2(1,0), rcpFrame.xy).a;
    float lumaSW = FxaaTexOff(tex, offUv, FxaaInt2(0,1), rcpFrame.xy).a;
    float lumaSE = FxaaTexOff(tex, offUv, FxaaInt2(1,1), rcpFrame.xy).a;
    float lumaM  = FxaaTexLod0(tex, uv).a;
/*---------------------------------------------------------*/
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
/*---------------------------------------------------------*/
    vec2 dir; 
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
/*---------------------------------------------------------*/
    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(FxaaFloat2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX), 
          max(FxaaFloat2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), 
          dir * rcpDirMin)) * rcpFrame.xy;
/*--------------------------------------------------------*/
    vec4 rgbA = (1.0/2.0) * (
        FxaaTexLod0(tex, uv + dir * (1.0/3.0 - 0.5)) +
        FxaaTexLod0(tex, uv + dir * (2.0/3.0 - 0.5)));
    vec4 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        FxaaTexLod0(tex, uv + dir * (0.0/3.0 - 0.5)) +
        FxaaTexLod0(tex, uv + dir * (3.0/3.0 - 0.5)));
    if((rgbB.a < lumaMin) || (rgbB.a > lumaMax)) return rgbA.xyz;
    return rgbB.xyz;
}

void main()
{
	vec3 color = fxaa(tonemapImage, v_uv, rcpTexSize);
	o_color = vec4(color, 1.0);
}

@end

@program Fxaa vs fs


