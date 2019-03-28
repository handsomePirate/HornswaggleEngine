#version 330 core
#extension GL_NV_shadow_samplers_cube: enable

in vec4 position;
flat in vec3 flatNorm;
in vec3 norm;
in vec3 tang;
in vec3 color;
in vec2 coords;
mat3 TBN;

struct Material
{
  float roughness;
  float metalness;

  vec3 color;
};

vec3 material_color;
vec3 diffuse_color;
vec3 specular_color;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
//uniform samplerCube cubemap;
uniform sampler2D environment_map;

uniform vec3 lightPositions[20]; // max number of lights in the scene
uniform vec3 lightColors[20];
uniform float lightIntensities[20];

uniform int lightsCount;

uniform vec3 camera;

uniform bool useTexture;

uniform Material material;

layout(location = 0) out vec4 fragColor;

const float PI = 3.14159265358979323846;

vec3 normal;
vec3 wi;

float last = (pow(norm.x, 2) * 4919 + pow(norm.z, 2) * 8737 + pow(norm.y, 2) * 6911) * 9883;

float rand()
{
	last = abs(468463 * int(last) + 11251) % 1654897;
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
 
    return vec3(cos(phi) * r, r1, sin(phi) * r);
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

vec4 brdf(vec3 wi, vec3 wo, float cos_theta, vec3 n, vec3 h_t)
{
	float roughness_2 = pow(material.roughness, 2);
	float r = (roughness_2 - 1) * pow(cos_theta, 2) + 1;
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

	vec3 reflectance = F * D * G / abs(n_v * n_l * 4);

	return vec4(reflectance * specular_color, D); // this might need to be f
}

float pdf(float D, float cos_theta)
{
	return max(D * cos_theta, 0.00000001);
}

vec3 sample_specular(int k)
{
	vec3 wo = normalize(lightPositions[k] - position.xyz);

	vec3 half_normal = normalize(wi + wo);
	float cos_theta = max(0, dot(half_normal, normal));

	vec4 res = brdf(wo, wi, cos_theta, h, half_normal);
	vec3 brdf_color = res.rgb;

	float distance = length(lights[k].position - position);
	float pdf = pdf(res.a, max(0, dot(normal, wi)));

	return (brdf_color * lightColors[k] * dot(normal, wi) * lightIntensities[k]) / (distance * distance) / pdf;
}

vec3 sample_diffuse(int k)
{
	float distance = length(lightPositions[k] - position.xyz);

	return (diffuse_color / PI * lightColors[k] * dot(normal, wi) * lightIntensities[k]) / (distance * distance);
}

vec2 ray_to_uv(vec3 direction)
{
	float cos_theta = direction.y;
	float theta = acos(cos_theta);

	if (direction.x == 0)
		direction.x = 0.001;

	float tan_phi = direction.z / direction.x;
	float phi = atan(tan_phi);
	
	if (direction.x < 0)
		phi += PI;

	phi += PI / 2;

	float map_x = (phi) / (2 * PI);
	float map_y = theta / PI;

	return vec2(map_x, map_y);
}

vec3 sample_brdf_environment()
{
	vec3 half_vector = sample_halfvector_GGX();
	vec3 half_vector_transformed = normalize(TBN * half_vector);
	//return half_vector_transformed;

	vec3 wo = reflect(wi, half_vector_transformed);
	//fragColor = vec4(dot(half_vector_transformed, wi), 0, 0, 1);

	vec4 res = brdf(wi, wo, half_vector.y, normal, half_vector_transformed);
	vec3 brdf_color = res.rgb;
	vec4 tex = texture(environment_map, ray_to_uv(wo));
	vec3 tex_color = tex.rgb / (tex.a * tex.a);

	return brdf_color * tex_color * dot(normal, wo) / pdf(res.a, max(0, dot(normal, wi)));
}

vec3 sample_diffuse_environment()
{
	vec3 half_vector = sample_uniform();
	vec3 half_vector_transformed = normalize(TBN * half_vector);
	vec3 wo = reflect(wi, half_vector_transformed);
	
	vec4 tex = texture(environment_map, ray_to_uv(wo));
	vec3 tex_color = tex.rgb;
	//return half_vector;
	
	return  diffuse_color / PI * tex_color;
}

vec4 get_color(int samples)
{
	vec3 color;

	for (int i = 0; i < lightsCount; ++i)
	{
		color += sample_specular(i);
		color += sample_diffuse(i);
	}

	vec3 environment_color;
	for (int i = 0; i < samples; ++i)
	{
		environment_color += sample_brdf_environment();
		environment_color += sample_diffuse_environment();
	}
	return vec4(color + environment_color / samples, 1);
}

void main(void)
{
	//fragColor = vec4(abs(normalize(tang)), 1);
	//return;

	material_color = material.color;
	normal = normalize(norm);
	vec3 tangent = normalize(tang);
	vec3 bitangent = cross(normal, tangent);
	TBN = mat3(tangent, normal, bitangent);

	if (useTexture)
	{
		float normal_strength = 1;

		normal = texture(normalMap, coords).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = mix(vec3(0, 0, 1), normal, normal_strength);

		normal = normalize(TBN * normal);

		material_color = texture(diffuseMap, coords).rgb;
	}
	diffuse_color = mix(material_color, vec3(0), material.metalness);
	specular_color = mix(vec3(1), material_color, material.metalness);

	wi = normalize(position.xyz - camera);
	//get_color(20);
	fragColor = get_color(20);
}