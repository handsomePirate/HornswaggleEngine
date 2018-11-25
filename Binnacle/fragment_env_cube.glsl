#version 330 core
#extension GL_NV_shadow_samplers_cube: enable

struct Material
{
  float ambience_c;
  float diffuse_c;
  float specular_c;

  float shininess;

  vec3 color;
};

in vec4 position;
flat in vec3 flatNorm;
in vec3 norm;
in vec3 color;
in vec2 coords;
in mat3 TBN;

uniform samplerCube cubemap;

uniform vec3 lightPositions[20]; // max number of lights in the scene
uniform vec3 lightColors[20];
uniform float lightIntensities[20];

uniform int lightsCount;

uniform vec3 camera;

uniform bool useTexture;
uniform vec3 matColor;

uniform Material material;

//out vec4 fragColor;

layout(location = 0) out vec4 fragColor;

void main(void)
{
	fragColor = vec4(textureCube(cubemap, -norm));
}