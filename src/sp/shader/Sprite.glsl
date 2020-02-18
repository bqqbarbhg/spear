@module Sprite

@vs vs

uniform Transform
{
	mat4 transform;
};

in vec2 position;
in vec2 texCoord;
in vec4 color;

out vec4 v_color;
out vec2 v_texCoord;

void main()
{
    gl_Position = transform * vec4(position, 0.0, 1.0);
	v_color = color;
	v_texCoord = texCoord;
}

@end

@fs fs

uniform sampler2D atlasTexture;

in vec4 v_color;
in vec2 v_texCoord;

out vec4 frag_color;

void main()
{
    frag_color = texture(atlasTexture, v_texCoord) * v_color;
}

@end

@program Sprite vs fs
