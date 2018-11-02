#version 330 core
#extension GL_NV_shadow_samplers_cube: enable

in vec4 position;
flat in vec3 norm;
in vec3 varNorm;
in vec3 color;
in vec2 coords;
in mat3 TBN;

struct Material
{
  float ambience_c;
  float diffuse_c;
  float specular_c;

  float shininess;

  vec3 specular_color;
  vec3 color;
};

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform samplerCube cubemap;

uniform vec3 lightPositions[20]; // max number of lights in the scene
uniform vec3 lightColors[20];
uniform float lightDiffI[20];
uniform float lightSpecI[20];

uniform int lightsCount;

uniform vec3 camera;

uniform bool useTexture;

uniform Material material;

layout(location = 0) out vec4 fragColor;

float brdf(vec3 wi, vec3 wo)
{
	// compute D 
	// compute F 
	// compute G

	float reflectance = 0;//F * D * G / (dot(varNorm, wi) * dot(varNorm, wo) * 4);

	return reflectance * material.specular_color;
}

void main(void)
{
	//fragColor = vec4(1, 0, 0, 1);
	//return;

	vec3 normal = varNorm;

	vec4 diffuseColor = vec4(material.color, 1);

	if (useTexture)
	{
		float normal_strength = 1;

		normal = texture(normalMap, coords).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = mix(vec3(0, 0, 1), normal, normal_strength);

		normal = normalize(TBN * normal);

		diffuseColor = texture(diffuseMap, coords);
	}

	vec4 diff_color = diffuseColor * vec4(textureCube(cubemap, reflect(position.xyz - camera, normal)));
	fragColor = clamp(diff_color, 0, 1);

	const int N = 10;
	vec3 color = vec3(0, 0, 0);

	for (int i = 0; i < N; ++i)
	{
		
	}
}