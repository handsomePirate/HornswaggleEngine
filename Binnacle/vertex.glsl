#version 330 core

in vec4 vertexPosition;
//in vec3 normal;
//in vec3 vertexColor;
//in vec2 vertexCoords;

//out vec3 color;
//out vec3 norm;
out vec4 position;
//out vec2 coords;
//out float texUnit;

uniform mat4 projectionViewMatrix;
uniform vec3 camera;

void main(void)
{
	// Setting up parameters for fragment shader
	//color = vertexColor;
	//norm = normal;
	// Determining vertex position
    
	gl_Position = projectionViewMatrix * vertexPosition;
	//gl_Position = vertexPosition;
	
	//coords = vertexCoords;
	//texUnit = textureUnit;
}