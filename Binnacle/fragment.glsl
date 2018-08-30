#version 330 core

in vec4 position;
in vec3 norm;
in vec3 color;
in vec2 coords;

uniform sampler2D texSampler;
uniform vec3 lights[256]; // max number of lights in the scene
uniform int lightsCount;

uniform vec3 camera;

uniform bool useTexture;
uniform vec3 matColor;

out vec4 fragColor;

void main(void)
{
	vec4 diffuse;
	// Sampling from texture
	if (useTexture)
		diffuse = texture(texSampler, coords);
	else
		diffuse = vec4(matColor, 1.0);

	//diffuse = vec4(vec3(1.0, 1.0, 1.0) - diffuse.rgb, diffuse.a);

	const float ambience_c = 0.2;
	const float diffuse_c = 1.0;
	const float specular_c = 0.2;

	float c_sum = ambience_c + diffuse_c + specular_c + 0.005;

	float ka = ambience_c / c_sum;
	float kd = diffuse_c / c_sum;
	float ks = specular_c / c_sum;

	vec3 occlusion_factor = vec3(0.95, 0.9, 1.0);
	vec3 occlusion_color = vec3(occlusion_factor.x * diffuse.x, occlusion_factor.y * diffuse.y, occlusion_factor.z * diffuse.z);
	vec3 Ea = (ka * occlusion_color).rgb;

	vec3 E_sum = vec3(0, 0, 0);
		
	for (int i = 0; i < lightsCount; ++i)
	{
		float Id = 40; // diffuse intensity
		float Is = 20; // specular intensity

		vec3 L = normalize(lights[i] - position.xyz);
		vec3 R = normalize(2 * norm * dot(norm, L) - L);
		vec3 V = normalize(camera - position.xyz);
		
		float cosa = dot(L, norm);
		if (cosa < 0) cosa = 0;
		vec3 Ed = (kd * diffuse * Id * cosa).rgb;
		
		float cosb = dot(R, V);
		if (cosb < 0) cosb = 0;

		float coshb = pow(cosb, 500);
		//fragColor = vec4(coshb, 0.0, 0.0, 1.0);
		//fragColor = vec4(abs(L), 1.0);
		//return;

		vec3 Es = ks * vec3(1, 1, 1) * Is * coshb;

		float c0 = 1;
		float c1 = 1;
		float c2 = 1;

		float length = length(position.xyz - lights[i]);
		E_sum += (Ed + Es) / (c0 + c1 * length + c2 * pow(length, 2));

		//fragColor = vec4(L, 1.0);
	}

	vec3 result = Ea + E_sum;

	fragColor = vec4(result, 1.0);
}