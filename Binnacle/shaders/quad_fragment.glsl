#version 330 core

in vec2 texcoord;

uniform sampler2D tex;
uniform int samples;

layout(location = 0) out vec4 fragColor;

void main(void)
{
	fragColor = texture(tex, vec2(texcoord.x, texcoord.y / float(samples)));
	for (int i = 1; i < samples; ++i)
	{
		fragColor = texture(tex, vec2(texcoord.x, (texcoord.y + i) / float(samples)));
	}
	fragColor /= float(samples);
}