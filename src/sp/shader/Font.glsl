@module Font

@vs vs

uniform Transform
{
	mat4 transform;
};

layout(location=0) in vec2 position;
layout(location=1) in vec2 texCoord;
layout(location=2) in vec4 color;
layout(location=3) in vec4 params;

out vec4 v_color;
out vec2 v_range;
out vec2 v_texCoord;

void main()
{
    gl_Position = transform * vec4(position, 0.0, 1.0);
	v_color = color;
	v_texCoord = texCoord;
	v_range = params.xy;
}

@end

@fs fs

uniform sampler2D atlasTexture;

in vec4 v_color;
in vec2 v_range;
in vec2 v_texCoord;

out vec4 frag_color;

void main()
{
	float sdf = texture(atlasTexture, v_texCoord).x;
	vec2 uvWidth = fwidth(v_texCoord);
	float radius = v_range.x;
	float width = v_range.y * 255.0 * max(uvWidth.x, uvWidth.y);
	float alpha = smoothstep(radius - width, radius + width, sdf);
    frag_color = alpha * v_color;
}

@end

@program Font vs fs

