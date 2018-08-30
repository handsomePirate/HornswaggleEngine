#version 330 core

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertexColor;
layout(location = 3) in vec2 vertexCoords;

smooth out vec3 color;
flat out vec3 norm;
smooth out vec3 varNorm;
smooth out vec4 position;
smooth out vec2 coords;
//out float texUnit;

uniform mat4 projectionViewMatrix;
uniform vec3 camera;

void main(void)
{
	// Setting up parameters for fragment shader
	color = vertexColor;
	norm = normal;
	varNorm = normal;
	//color = vec3(vertexCoords, 0);//normal;
	
	// Determining vertex position
	gl_Position = projectionViewMatrix * vertexPosition; 
	position = vertexPosition;
	//gl_Position = vertexPosition;
	
	coords = vertexCoords;
	//texUnit = textureUnit;
}