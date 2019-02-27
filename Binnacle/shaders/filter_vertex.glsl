#version 330 core

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertexColor;
layout(location = 3) in vec2 vertexCoords;
layout(location = 4) in vec3 tangent;

out vec2 uv;
out vec4 position;

void main(void)
{
	uv = vertexCoords;
	gl_Position = vertexPosition;
}