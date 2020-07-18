@module Particle

@ctype vec2 sf::Vec2
@ctype vec3 sf::Vec3
@ctype vec4 sf::Vec4

@vs vs

layout(location=0) in vec4 a_PositionLife;
layout(location=1) in vec4 a_VelocitySeed;

out vec4 v_Color;

uniform Vertex
{
	mat4 u_WorldToClip;
	float u_InvDelta;
};

void main()
{
	uint id = uint(gl_VertexID);
	float invDelta = u_InvDelta;

	vec3 position = vec3(a_PositionLife.xyz);
	vec3 velocity = vec3(a_VelocitySeed.xyz);
	float life = a_PositionLife.w + invDelta;

	position -= velocity * invDelta;

	vec4 ndc = u_WorldToClip * vec4(position, 1.0);
	ndc.x += (float(id & 1) * 2.0 - 1.0) * 0.05;
	ndc.y += (float((id >> 1) & 1) * 2.0 - 1.0) * 0.05;

	if (life <= 0.0) {
		ndc = vec4(0.0);
	}

	gl_Position = ndc;
	v_Color = vec4(1.0) * clamp(life, 0.0, 1.0);
}

@end

@fs fs

in vec4 v_Color;

out vec4 o_color;

void main()
{
	o_color = v_Color;
}

@end

@program Particle vs fs

