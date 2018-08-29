#version 330 core

in vec4 position;
in vec3 norm;
in vec3 color;
in vec2 coords;

uniform sampler2D texSampler;
uniform vec3 lights[];
uniform bool useTexture;
uniform vec3 matColor;

out vec4 fragColor;

void main(void)
{
	// Sampling from texture
	if (useTexture)
		fragColor = texture(texSampler, coords);
	else
		fragColor = vec4(matColor, 1.0);
}