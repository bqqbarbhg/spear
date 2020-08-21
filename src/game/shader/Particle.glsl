@module Particle

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

layout(location=0) in vec4 a_PositionLife;
layout(location=1) in vec4 a_VelocitySeed;

uniform sampler2D u_SplineTexture;

out vec2 v_Uv0;
out vec2 v_Uv1;

flat out vec4 v_Color;
flat out vec4 v_Params;

uniform VertexInstance
{
	mat4 u_ParticleToWorld;
	mat4 u_WorldToClip;
	float u_InvDelta;
	float u_Aspect;
};

uniform VertexType
{
	vec2 u_FrameCount;
	float u_FrameRate;
	vec4 u_RotationControl;
	float u_Additive;
	float u_StartFrame;
	vec4 u_SplineMad;
	vec2 u_ScaleBaseVariance;
	vec2 u_LifeTimeBaseVariance;
	float u_RelativeFrameRate;
};

float evaluateRotation(vec4 control, float t, vec2 seed)
{
	float base = control.x + control.y * (seed.x * 2.0 - 1.0);
	float spin = control.z + control.w * (seed.y * 2.0 - 1.0);
	return base + spin * t;
}

float srgbToLinear(float x)
{
	x = clamp(x, 0.0, 1.0);
	if (x <= 0.04045)
		return 0.0773993808 * x;
	else
		return pow(x*0.947867298578+0.0521327014218, 2.4);
}

vec3 srgbToLinear(vec3 v)
{
	return vec3(srgbToLinear(v.x), srgbToLinear(v.y), srgbToLinear(v.z));
}

void main()
{
	uint id = uint(gl_VertexID);
	float invDelta = u_InvDelta;

	vec3 position = (u_ParticleToWorld * vec4(a_PositionLife.xyz, 1.0)).xyz;
	vec3 velocity = (u_ParticleToWorld * vec4(a_VelocitySeed.xyz, 0.0)).xyz;
	float packedSeed = a_VelocitySeed.w;

	float absoluteTotalLifeTime = u_LifeTimeBaseVariance.x + u_LifeTimeBaseVariance.y * packedSeed;
	float lifeLeft = a_PositionLife.w + invDelta / absoluteTotalLifeTime;
	float lifeTime = 1.0 - lifeLeft;
	float absoluteLifeTime = lifeTime * absoluteTotalLifeTime;

	vec4 seed = fract(floor(packedSeed * vec4(1.0, 1.0/64.0, 1.0/4096.0, 1.0/262144.0)) * (1.0 / 64.0));

	position -= velocity * invDelta;

	vec2 uv = vec2(float(id & 1), float((id >> 1) & 1));
	vec2 offset = (uv * 2.0 - 1.0);

	vec2 splineUv = vec2(clamp(lifeTime, 0.0, 1.0), 0.0) * u_SplineMad.xy + u_SplineMad.zw;

	vec4 spline = texture(u_SplineTexture, splineUv);
	vec4 gradient = texture(u_SplineTexture, splineUv + vec2(0.0, u_SplineMad.y));

	vec3 gradientColor = srgbToLinear(gradient.xyz);

	float scale = u_ScaleBaseVariance.x + seed.x*u_ScaleBaseVariance.y;
	scale *= spline.x;
	offset *= max(scale, 0.0);

	float rotation = evaluateRotation(u_RotationControl, absoluteLifeTime, seed.zw);
	vec2 axisX = vec2(cos(rotation), sin(rotation));
	vec2 axisY = vec2(axisX.y, -axisX.x);

	vec4 ndc = u_WorldToClip * vec4(position, 1.0);

	ndc.xy += (offset.x * axisX + offset.y * axisY) * vec2(0.5 * u_Aspect, 0.5);

	if (lifeLeft <= 0.0) {
		ndc = vec4(0.0);
	}

	float alpha = spline.y;

	float frameRate = mix(absoluteLifeTime, lifeTime, u_RelativeFrameRate);
	float frame = frameRate * u_FrameRate + floor(seed.x*64.0 + seed.z*4096) * u_StartFrame;
	float frameI = floor(frame);
	float frameD = frame - frameI;

	vec2 frame0 = vec2(mod(frameI, u_FrameCount.x), mod(floor(frameI / u_FrameCount.x), u_FrameCount.y));
	vec2 frame1 = vec2(mod(frameI + 1, u_FrameCount.x), mod(floor((frameI + 1) / u_FrameCount.x), u_FrameCount.y));

	vec4 color = vec4(gradientColor, alpha);
	float erode = spline.w < 1.0 ? 1.0 / max(spline.w, 0.01) : 1.0;

	gl_Position = ndc;
	v_Color = color;
	v_Uv0 = (frame0 + uv) / u_FrameCount;
	v_Uv1 = (frame1 + uv) / u_FrameCount;
	v_Params.x = frameD;
	v_Params.y = spline.z;
	v_Params.z = erode;
	v_Params.w = 0.0;
}

@end

@fs fs

uniform sampler2D u_Texture;

in vec2 v_Uv0;
in vec2 v_Uv1;

flat in vec4 v_Color;
flat in vec4 v_Params;

out vec4 o_color;

void main()
{
	vec4 tex0 = texture(u_Texture, v_Uv0);
	vec4 tex1 = texture(u_Texture, v_Uv1);

	float frameBlend = v_Params.x;
	float additive = v_Params.y;
	float erode = v_Params.z;

	vec4 tex = mix(tex0, tex1, frameBlend);
	tex.xyz *= v_Color.xyz;
	if (erode > 1.0) {
		tex.w = clamp((tex.w + v_Color.w - 1.001) * erode, 0.0, 1.0);
	} else {
		tex.w *= v_Color.w;
	}

	tex.xyz *= tex.w;
	tex.w *= additive;

	o_color = tex;
}

@end

@program Particle vs fs

