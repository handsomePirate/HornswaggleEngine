#version 300

in vec3 vertexPosition;
in vec3 normal;
in vec3 vertexColor;
in vec2 vertexCoords;

out vec3 color;
out vec3 norm;
out vec3 position;
out vec2 coords;
out float texUnit;

uniform mat4 projectionViewMatrix;

void main(void)
{
	// Determining vertex position
    gl_Position = projectionViewMatrix * vec4(vertexPosition, 1.0);

	// Setting up parameters for fragment shader
	color = vertexColor;
	norm = normal;
	position = vertexPosition;
	coords = vertexCoords;
	//texUnit = textureUnit;
}