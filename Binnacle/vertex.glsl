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

//uniform mat4 projectionViewMatrix;

void main(void)
{
	// Determining vertex position
    //gl_Position = projectionViewMatrix * vec4(vertexPosition, 1.0);

	// Setting up parameters for fragment shader
	//color = vertexColor;
	//norm = normal;
	gl_Position = vertexPosition;
	position = vertexPosition;
	//coords = vertexCoords;
	//texUnit = textureUnit;
}