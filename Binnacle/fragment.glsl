#version 330 core

struct Material
{
  float ambience_c;
  float diffuse_c;
  float specular_c;

  float shininess;

  vec3 color;
};

in vec4 position;
flat in vec3 norm;
in vec3 varNorm;
in vec3 color;
in vec2 coords;
in mat3 TBN;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform vec3 lightPositions[20]; // max number of lights in the scene
uniform vec3 lightColors[20];
uniform float lightDiffI[20];
uniform float lightSpecI[20];

uniform int lightsCount;

uniform vec3 camera;

uniform bool useTexture;
uniform vec3 matColor;

uniform Material material;

//out vec4 fragColor;

layout(location = 0) out vec4 fragColor;

void main(void)
{
	fragColor = vec4(0.5, 0.5, 0.5, 1);
}