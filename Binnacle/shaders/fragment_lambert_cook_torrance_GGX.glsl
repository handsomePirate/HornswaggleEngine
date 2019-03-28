#version 330 core

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

vec3 material_diffuse_color;
vec3 material_specular_color;
vec3 diffuse_color;
vec3 specular_color;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
//uniform samplerCube cubemap;
uniform sampler2D environment_map;
uniform sampler2D environment_map_diffuse;

uniform vec3 lightPositions[20]; // max number of lights in the scene
uniform vec3 lightColors[20];
uniform float lightIntensities[20];

uniform int lightsCount;

uniform vec3 camera;
vec3 normal;

uniform bool useTexture;

uniform Material material;

uniform int windowWidth;
uniform int windowHeight;

int index;

vec2 pix;
vec3 wo;

layout(location = 0) out vec4 fragColor;

const float PI = 3.14159265358979323846;

//==================================RANDOM========================================
bool rand_init = true;
uvec2 rand_stage;
#define TEA_ITERATION_COUNT 32u
uvec2 tea(uvec2 v)
{
	uint k[4];
	{ 
		k[0] = 0xA341316Cu;
		k[1] = 0xC8013EA4u;
		k[2] = 0xAD90777Du;
		k[3] = 0x7E95761Eu;
	}
	uint sum = uint(0);
	const uint delta = 0x9E3779B9u;

	for (uint i = uint(0); i < TEA_ITERATION_COUNT; ++i)
	{
		sum += delta;
		v[0] += ((v[1] << 4) + k[0]) & (v[1] + sum) & ((v[1] >> 5) + k[1]);
		v[1] += ((v[0] << 4) + k[2]) & (v[0] + sum) & ((v[0] >> 5) + k[3]);
	}
	return v;
}

vec2 rand()
{
	if (rand_init)
	{
		rand_init = false;
		rand_stage = tea(uvec2(abs(pix.x), abs(pix.y)));
		return rand_stage / float(uint(0xFFFFFFFF));
	}
	rand_stage = tea(rand_stage);
	return rand_stage / float(uint(0xFFFFFFFF));
}
//================================================================================

vec3 sample_uniform()
{
	vec2 rv = rand();

	float r = sqrt(1.0f - rv[0] * rv[0]);
    float phi = 2 * PI * rv[1];
 
    return vec3(cos(phi) * r, rv[0], sin(phi) * r);
}

vec3 halfvector_GGX_sample()
{
	vec2 rv = rand();
	
	rv[0] = min(rv[0], 0.9999);

	float theta = atan(material.roughness * sqrt(rv[0] / (1 - rv[0])));
	float phi = 2 * PI * rv[1];

	float x = sin(theta) * cos(phi);
	float z = sin(theta) * sin(phi);
	float y = cos(theta);

	return vec3(x, y, z);
}

vec2 ray_to_uv(vec3 direction)
{
	float cos_theta = direction.y;
	float theta = acos(cos_theta);

	//if (direction.x == 0)
	//	direction.x = 0.001;

	float tan_phi = direction.z / direction.x;
	float phi = atan(tan_phi);
	
	if (direction.x < 0)
		phi += PI;

	//phi += PI / 2;

	float map_x = (phi) / (2 * PI);
	float map_y = theta / PI;

	return vec2(map_x, map_y);
}
//=========================================================================

int grey_code(int n)
{
	return n ^ (n - 1);
}

float attenuation(float distance)
{
	return distance * distance;
}

float computeD(float cos_theta)
{
	float roughness_2 = material.roughness * material.roughness;
	float r = (roughness_2 - 1) * cos_theta * cos_theta + 1;
	float D = roughness_2 / (PI * r * r);
	if (isnan(D))
		D = 1;
	return D;
}

float shadowing_schlick(float roughness, float d)
{
	float k = roughness * sqrt(2 / PI);
	return d / (d * (1 - k) + k);
}

float shadowing_smith(float roughness, float n_v, float n_l)
{
	return shadowing_schlick(roughness, n_v) * shadowing_schlick(roughness, n_l);
}

vec4 brdf(vec3 wo, vec3 wi, vec3 h_t)
{
	float cos_theta = dot(h_t, normal);
	float D = computeD(cos_theta);

	float v_h = dot(h_t, wi);
	vec3 f = vec3(0.04);
	f = mix(f, material.color, material.metalness);
	vec3 F = f + (1 - f) * pow(1 - v_h, 5);

	float n_v = abs(dot(normal, wo));
	float n_l = abs(dot(normal, wi));
	float G = shadowing_smith(material.roughness, n_v, n_l);

	vec3 reflectance = F * D * G / abs(n_v * n_l * 4);

	return vec4(reflectance * material_specular_color, D); // this might need to be f
}

float pdf(float D, float cos_theta)
{
	return max(D / (4 * cos_theta), 0.000001);
}

vec3 point_lights_cook_torrance()
{
	vec3 contributions = vec3(0, 0, 0);
	for (int i = 0; i < lightsCount; ++i)
	{
		vec3 wi = normalize(position.xyz - lightPositions[i]);
		float r = length(position.xyz - lightPositions[i]);
		float cos_theta = max(0, dot(-wi, normal));

		contributions += lightColors[i] * lightIntensities[i] * cos_theta / attenuation(r);
	}
	return contributions * material_specular_color / PI;
}

vec3 point_lights_lambert()
{
	vec3 contributions = vec3(0, 0, 0);
	for (int i = 0; i < lightsCount; ++i)
	{
		vec3 wi = normalize(position.xyz - lightPositions[i]);
		float r = length(position.xyz - lightPositions[i]);
		float cos_theta = max(0, dot(-wi, normal));

		contributions += lightColors[i] * lightIntensities[i] * cos_theta / attenuation(r);
	}
	return contributions * material_diffuse_color / PI;
}

vec3 environment_map_cook_torrance(int samples)
{
	vec3 contributions = vec3(0, 0, 0);
	for (int i = 0; i < samples; ++i)
	{
		vec3 half_vector = halfvector_GGX_sample();
		vec3 half_vector_transformed = normalize(TBN * half_vector);

		vec3 wi = reflect(wo, half_vector_transformed);
		
		vec4 res = brdf(wo, wi, half_vector_transformed);
		vec3 brdf_color = res.rgb;
		vec4 tex = texture(environment_map, ray_to_uv(-wi));
		vec3 tex_color = tex.rgb / (tex.a * tex.a);
		
		contributions += brdf_color * tex_color * dot(normal, -wi) / pdf(res.a, max(0, dot(normal, -wi)));
	}
	// Average all the contributions
	return contributions / samples;
}

vec3 environment_map_lambert(int samples)
{
	// For now use sampling => uniformly sample over the hemisphere
	vec3 contributions = vec3(0, 0, 0);
	for (int i = 0; i < samples; ++i)
	{
		vec3 half_vector = sample_uniform();
		vec3 half_vector_transformed = normalize(TBN * half_vector);
		vec3 wi = reflect(wo, half_vector_transformed);
	
		vec4 tex = texture(environment_map, ray_to_uv(wi));
		vec3 tex_color = tex.rgb;
		//vec3 tex_color = texture(environment_map_diffuse, ray_to_uv(normal)).rgb;
		
		contributions += tex_color;
	}
	// Average all the contributions
	return contributions / samples * material_diffuse_color;
}

vec3 specular_brdf()
{
	//return vec3(0);
	return /*point_lights_cook_torrance() + */environment_map_cook_torrance(20);
}

vec3 diffuse_brdf()
{
	return vec3(0);
	return point_lights_lambert() + environment_map_lambert(20);
}

void prepare_surface_data()
{
	normal = normalize(norm);
	vec3 tangent = normalize(tang);
	vec3 bitangent = cross(normal, tangent);
	TBN = mat3(tangent, normal, bitangent);

	material_diffuse_color = material.color;

	if (useTexture)
	{
		float normal_strength = 1;

		normal = texture(normalMap, coords).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = mix(vec3(0, 0, 1), normal, normal_strength);

		normal = normalize(TBN * normal);

		material_diffuse_color = texture(diffuseMap, coords).rgb;
	}

	material_diffuse_color = mix(material_diffuse_color, vec3(0), material.metalness);
	material_specular_color = mix(vec3(1), material.color, material.metalness); // TODO: exchange for specular material color

	const int mult_constant = 100;
}

void main(void)
{
	pix = gl_FragCoord.xy;
	prepare_surface_data();
	
	wo = normalize(camera - position.xyz);
	vec3 diffuse_color = diffuse_brdf();
	vec3 specular_color = specular_brdf();
	fragColor = vec4(diffuse_color + specular_color, 1);
}