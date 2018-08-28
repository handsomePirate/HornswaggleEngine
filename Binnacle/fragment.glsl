#version 330 core

in vec4 position;
//in vec3 norm;
//in vec3 color;
//in vec2 coords;

//uniform sampler2D diffuseSampler;

out vec4 fragColor;

void main(void)
{
	// Sampling from texture
	// fragColor = vec4(texture(diffuseSampler, coords).rgb, 1);
	fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}