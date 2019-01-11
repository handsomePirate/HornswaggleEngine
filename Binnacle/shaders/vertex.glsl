#version 330 core

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertexColor;
layout(location = 3) in vec2 vertexCoords;
layout(location = 4) in vec3 tangent;

out vec3 color;
flat out vec3 flatNorm;
out vec3 norm;
out vec3 tang;
out vec4 position;
out vec2 coords;

uniform mat4 projectionViewMatrix;
uniform vec3 camera;

void main(void)
{
	// Setting up parameters for fragment shader
	color = vertexColor;
	norm = normal;
	flatNorm = normal;
	tang = tangent;
	
	// Determining vertex position
	gl_Position = projectionViewMatrix * vertexPosition; 
	position = vertexPosition;
	//gl_Position = vertexPosition;
	
	coords = vertexCoords;
	//texUnit = textureUnit;
}