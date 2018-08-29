#version 330 core

in vec4 position;
in vec3 norm;
in vec3 color;
in vec2 coords;

uniform sampler2D texSampler;
uniform vec3 lights[];

out vec4 fragColor;

void main(void)
{
	// Sampling from texture
	fragColor = texture(texSampler, coords);
	//fragColor = vec4(coords, 1.0, 1.0);
}