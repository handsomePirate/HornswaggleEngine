#version 330 core
#extension GL_NV_shadow_samplers_cube: enable

in vec4 position;
flat in vec3 norm;
in vec3 varNorm;
in vec3 tang;
in vec3 color;
in vec2 coords;
mat3 TBN;

struct Material
{
  float ambience_c;
  float diffuse_c;
  float specular_c;

  float roughness;
  float metalness;

  vec3 specular_color;
  vec3 color;
};

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform samplerCube cubemap;

uniform vec3 lightPositions[20]; // max number of lights in the scene
uniform vec3 lightColors[20];

uniform int lightsCount;

uniform vec3 camera;

uniform bool useTexture;

uniform Material material;

layout(location = 0) out vec4 fragColor;

const float PI = 3.14159265358979323846;

float last = (pow(varNorm.x, 2) * 4919 + pow(varNorm.z, 2) * 8737 + pow(varNorm.y, 2) * 6911) * 9883;

float rand()
{
	last = abs(468463 * int(last) + 11251) % 1654897;//2147483647
    return last / 1654897.0;
}

vec3 sample_halfvector_GGX()
{
	float r1 = rand();
	float r2 = rand();
	r1 = min(r1, 0.9999);

	float theta = atan(material.roughness * sqrt(r1 / (1 - r1)));
	float phi = 2 * PI * r2;

	float x = sin(theta) * cos(phi);
	float z = sin(theta) * sin(phi);
	float y = cos(theta);

	return vec3(x, y, z);
}

vec3 sample_uniform()
{
	float r1 = rand();
	float r2 = rand();

	float r = sqrt(1.0f - r1 * r1);
    float phi = 2 * PI * r2;
 
    return vec3(cos(phi) * r, sin(phi) * r, r1);
}

float shadowing_schlick(float d)
{
	float k = material.roughness * sqrt(2 / PI);
	return d / (d * (1 - k) + k);
}

float shadowing_smith(float n_v, float n_l)
{
	return shadowing_schlick(n_v) * shadowing_schlick(n_l);
}

vec4 brdf(vec3 wi, vec3 wo, vec3 h, vec3 n, vec3 h_t)
{
	float roughness_2 = pow(material.roughness, 2);
	float r = (roughness_2 - 1) * pow(h.y, 2) + 1;
	float D = roughness_2 / (PI * pow(r, 2));
	if (isnan(D))
		D = 1;

	float v_h = dot(h_t, wo);
	vec3 f = vec3(0.04);
	f = mix(f, material.color, material.metalness);
	vec3 F = f + (1 - f) * pow(1 - v_h, 5);

	float n_v = abs(dot(n, wi));
	float n_l = abs(dot(n, wo));
	float G = shadowing_smith(n_v, n_l);
	//fragColor = vec4(h.y, 0, 0, 1);
	//fragColor = vec4(F, 1);

	vec3 reflectance = F * D * G / abs(n_v * n_l * 4);

	//fragColor = vec4(D, D, D, 1);
	//fragColor = vec4(G, G, G, 1);
	//fragColor = vec4(F, 1);
	//fragColor = vec4(G * F, 1);

	return vec4(reflectance * material.specular_color, D);
}

vec3 normal;
vec3 wi;

float pdf(float D, vec3 h, vec3 wo)
{
	//float w_h = abs(dot(wo, h));
	//float p = D / (4 * w_h);
	//return max(p, 0.00000001);
	return max(D * h.y, 0.00000001);
}

vec3 sample_brdf()
{
	wi = normalize(position.xyz - camera);

	vec3 half_vector = sample_halfvector_GGX();
	vec3 half_vector_transformed = normalize(TBN * half_vector);
	//return half_vector_transformed;

	vec3 wo = reflect(wi, half_vector_transformed);
	//fragColor = vec4(dot(half_vector_transformed, wi), 0, 0, 1);

	vec4 res = brdf(wi, wo, half_vector, normal, half_vector_transformed);
	vec3 brdf_color = res.rgb;
	vec3 tex_color = textureCube(cubemap, wo).rgb;

	return (brdf_color * tex_color * dot(normal, wo) / pdf(res.a, half_vector, wo));
}

vec3 sample_diffuse()
{
	wi = normalize(position.xyz - camera);
	vec3 half_vector = sample_uniform();
	vec3 half_vector_transformed = normalize(TBN * half_vector);
	vec3 wo = reflect(wi, half_vector_transformed);
	vec3 tex_color = textureCube(cubemap, wo).rgb;
	//return half_vector;
	
	return  material.color / PI * tex_color;
}

vec4 get_color(int samples)
{
	vec3 color;

	for (int i = 0; i < samples; ++i)
	{
		color += sample_brdf();
		color += sample_diffuse();
	}
	return vec4(color / samples, 1);
}

void main(void)
{
	//fragColor = vec4(abs(normalize(tang)), 1);
	//return;

	vec3 tangent = normalize(tang);
	normal = normalize(varNorm);
	vec3 bitangent = cross(normal, tangent);
	TBN = mat3(tangent, normal, bitangent); // might need to switch normal and bitangent

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

	//float r1 = abs(rand());
	//float r2 = abs(rand());
	//float theta = atan(material.roughness * sqrt(r1 / (1 - r1)));
	//float phi = 2 * PI * r2;
	//float x = sin(theta) * cos(phi);
	//float z = sin(theta) * sin(phi);
	//float y = cos(theta);
	//
	//rand();
	//fragColor = vec4(rand(), 0, 0, 1);
	//if (fragColor.x < 0 || fragColor.y < 0|| fragColor.z < 0)
	//	fragColor = vec4(1);
	//return;

	//get_color(20);
	fragColor = get_color(20);
}