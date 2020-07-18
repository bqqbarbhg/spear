@module Particle

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

layout(location=0) in vec4 a_PositionLife;
layout(location=1) in vec4 a_VelocitySeed;

out vec4 v_Color;
out vec2 v_Uv0;
out vec2 v_Uv1;

flat out float v_FrameBlend;

uniform Vertex
{
	mat4 u_WorldToClip;
	float u_InvDelta;
	vec2 u_FrameCount;
	float u_FrameRate;
	float u_Aspect;
	vec4 u_ScaleAnim;
	vec4 u_AlphaAnim;
	vec4 u_RotationControl;
	float u_Additive;
	float u_StartFrame;
};

float evaluateAnim(vec4 control, float t, float seed)
{
	float a = clamp(t * control.z, 0.0, 1.0);
	float b = clamp((1.0 - t) * control.w, 0.0, 1.0);
	float sa = a * a * (3.0 - 2.0 * a);
	float sb = b * b * (3.0 - 2.0 * b);
	return a * b * (control.x + seed * control.y);
}

float evaluateRotation(vec4 control, float t, vec2 seed)
{
	float base = control.x + control.y * (seed.x * 2.0 - 1.0);
	float spin = control.z + control.w * (seed.y * 2.0 - 1.0);
	return base + spin * t;
}

void main()
{
	uint id = uint(gl_VertexID);
	float invDelta = u_InvDelta;

	vec3 position = vec3(a_PositionLife.xyz);
	vec3 velocity = vec3(a_VelocitySeed.xyz);
	float packedSeed = a_VelocitySeed.w;
	float lifeLeft = a_PositionLife.w + invDelta;
	float lifeTime = 1.0 - lifeLeft;

	vec4 seed = fract(floor(packedSeed * vec4(1.0, 1.0/64.0, 1.0/4096.0, 1.0/262144.0)) * (1.0 / 64.0));

	position -= velocity * invDelta;

	vec2 uv = vec2(float(id & 1), float((id >> 1) & 1));
	vec2 offset = (uv * 2.0 - 1.0);

	float scale = evaluateAnim(u_ScaleAnim, lifeTime, seed.x);
	offset *= max(scale, 0.0);

	float rotation = evaluateRotation(u_RotationControl, lifeTime, seed.zw);
	vec2 axisX = vec2(cos(rotation), sin(rotation));
	vec2 axisY = vec2(axisX.y, -axisX.x);

	vec4 ndc = u_WorldToClip * vec4(position, 1.0);

	ndc.xy += (offset.x * axisX + offset.y * axisY) * vec2(0.5 * u_Aspect, 0.5);

	if (lifeLeft <= 0.0) {
		ndc = vec4(0.0);
	}

	float alpha = evaluateAnim(u_AlphaAnim, lifeTime, seed.y);

	float frame = lifeTime * u_FrameRate + floor(seed.x*64.0 + seed.z*4096) * u_StartFrame;
	float frameI = floor(frame);
	float frameD = frame - frameI;

	vec2 frame0 = vec2(mod(frameI, u_FrameCount.x), mod(floor(frameI / u_FrameCount.x), u_FrameCount.y));
	vec2 frame1 = vec2(mod(frameI + 1, u_FrameCount.x), mod(floor((frameI + 1) / u_FrameCount.x), u_FrameCount.y));

	vec4 color = vec4(1.0) * clamp(alpha, 0.0, 1.0);

	color.a *= 1.0 - u_Additive;

	gl_Position = ndc;
	v_Color = color;
	v_Uv0 = (frame0 + uv) / u_FrameCount;
	v_Uv1 = (frame1 + uv) / u_FrameCount;
	v_FrameBlend = frameD;
}

@end

@fs fs

uniform sampler2D u_Texture;

in vec4 v_Color;
in vec2 v_Uv0;
in vec2 v_Uv1;
flat in float v_FrameBlend;

out vec4 o_color;

void main()
{
	vec4 tex0 = texture(u_Texture, v_Uv0);
	vec4 tex1 = texture(u_Texture, v_Uv1);

	tex0.xyz *= tex0.w;
	tex1.xyz *= tex1.w;

	vec4 tex = mix(tex0, tex1, v_FrameBlend);

	o_color = v_Color * tex;
}

@end

@program Particle vs fs

