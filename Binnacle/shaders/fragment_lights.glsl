#version 330 core

in vec4 position;
in vec3 color;

uniform vec3 camera;

layout(location = 0) out vec4 fragColor;

void main(void)
{
	fragColor = vec4(color, 1);
}