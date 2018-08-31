#version 330 core

in vec4 position;
flat in vec3 norm;
in vec3 varNorm;
in vec3 color;
in vec2 coords;

uniform sampler2D texSampler;

uniform vec3 lightPositions[20]; // max number of lights in the scene
uniform vec3 lightColors[20];
uniform float lightDiffI[20];
uniform float lightSpecI[20];

uniform int lightsCount;

uniform vec3 camera;

uniform bool useTexture;
uniform vec3 matColor;

//uniform bool smoothShading;

out vec4 fragColor;

void main(void)
{
	vec3 normal;
	normal = varNorm;

	vec3 diffuse_color;
	// Sampling from texture
	if (useTexture)
		diffuse_color = vec3(texture(texSampler, coords));
	else
		diffuse_color = matColor;

	const float ambience_c = 0.2;
	const float diffuse_c = 1.0;
	const float specular_c = 0.2;

	float c_sum = ambience_c + diffuse_c + specular_c + 0.005;

	float ka = ambience_c / c_sum;
	float kd = diffuse_c / c_sum;
	float ks = specular_c / c_sum;

	vec3 occlusion_factor = vec3(0.95, 0.9, 1.0);
	vec3 occlusion_color = vec3(occlusion_factor.x * diffuse_color.x, occlusion_factor.y * diffuse_color.y, occlusion_factor.z * diffuse_color.z);
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

		vec3 Ed = kd * diffuse_color * Id * cosa;
		
		float cosb = dot(R, V);

		if (cosb < 0) cosb = 0;

		float coshb = pow(cosb, 15);

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