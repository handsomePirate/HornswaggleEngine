#version 330 core

layout(location = 0) in vec4 vertexPosition;
layout(location = 2) in vec3 vertexColor;

out vec4 position;
out vec3 color;

uniform mat4 projectionViewMatrix;
uniform vec3 camera;

void main(void)
{
	// Setting up parameters for fragment shader
	color = vertexColor;

	// Determining vertex position
	gl_Position = projectionViewMatrix * vertexPosition; 
	position = vertexPosition;

	gl_PointSize = 8;
	//gl_Position = vertexPosition;	
}