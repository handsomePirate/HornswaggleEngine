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
	vec3 normal;
	normal = varNorm;

	vec3 diffuse_color;
	diffuse_color = material.color;
	// Sampling from texture

	if (useTexture)
	{
		float normal_strength = 1;

		normal = texture(normalMap, coords).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = mix(vec3(0, 0, 1), normal, normal_strength);

		normal = normalize(TBN * normal);

		diffuse_color = vec3(texture(diffuseMap, coords));
	}	

	//const float ambience_c = 0.2;
	//const float diffuse_c = 1.0;
	//const float specular_c = 0.2;

	float c_sum = material.ambience_c + material.diffuse_c + material.specular_c + 0.005;

	float ka = material.ambience_c / c_sum;
	float kd = material.diffuse_c / c_sum;
	float ks = material.specular_c / c_sum;

	vec3 occlusion_factor = vec3(0.95, 0.9, 1.0);
	vec3 occlusion_color = diffuse_color * occlusion_factor;
	vec3 Ea = (ka * occlusion_color).rgb;

	vec3 E_sum = vec3(0, 0, 0);
		
	for (int i = 0; i < lightsCount; ++i)
	{
		float Id = lightDiffI[i]; // diffuse intensity
		float Is = lightSpecI[i]; // specular intensity

		vec3 L = normalize(lightPositions[i] - position.xyz);
		vec3 R = normalize(2 * normal * dot(normal, L) - L);
		vec3 V = normalize(camera - position.xyz);
		
		float cosa = dot(L, normal);
		if (cosa < 0) cosa = 0;

		vec3 Ed = kd * (diffuse_color * lightColors[i]) * Id * cosa;
		
		float cosb = dot(R, V);

		if (cosb < 0) cosb = 0;

		float coshb = pow(cosb, material.shininess);

		vec3 Es = ks * lightColors[i] * Is * coshb;

		float c0 = 1;
		float c1 = 1;
		float c2 = 1;

		float length = length(position.xyz - lightPositions[i]);
		E_sum += (Ed + Es) / (c0 + c1 * length + c2 * pow(length, 2));
	}

	vec3 result = Ea + E_sum;

	fragColor = vec4(result, 1.0);
	//fragColor = vec4(abs(varNorm), 1.0);
}