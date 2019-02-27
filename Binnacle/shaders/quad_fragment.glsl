#version 330 core

in vec2 texcoord;

uniform sampler2D tex;

layout(location = 0) out vec4 fragColor;

void main(void)
{
	fragColor = texture(tex, texcoord);
}