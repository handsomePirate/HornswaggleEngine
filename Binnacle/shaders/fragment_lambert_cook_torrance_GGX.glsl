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
  float n;

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

#define PI 3.14159265358979323846
#define OneOverPI 0.31830988618379067153
#define SqrtTwoOverPI 0.7978845608028653559

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

float shadowingSchlick(float roughness, float d)
{
	float k = roughness * SqrtTwoOverPI;
	return d / (d * (1 - k) + k);
}

float shadowingSmith(float roughness, float n_v, float n_l)
{
	return shadowingSchlick(roughness, n_v) * shadowingSchlick(roughness, n_l);
}

float f0Fresnel()
{
	float f0 = (1 - material.n) / (1 + material.n);
	return f0 * f0;
}

float fresnelTerm(vec3 wi, vec3 microfacet)
{
	float f0 = f0Fresnel();
	return f0 + (1 - f0) * pow(1 - dot(wi, normal), 5);
}

float geometryTerm(vec3 wi, vec3 wo, vec3 microfacet)
{
	return shadowingSmith(material.roughness, max(0, dot(wo, normal)), max(0, dot(wi, normal)));
}

float normalDistributionFunction(vec3 microfacet)
{
	float cos_theta = dot(microfacet, normal);
	float roughness_2 = material.roughness * material.roughness;
	float r = (roughness_2 - 1) * cos_theta * cos_theta + 1;
	float D = roughness_2 / (PI * r * r);
	if (isnan(D))
		D = 1;
	return D;
}

vec2 brdf(vec3 wo, vec3 wi, vec3 microfacet)
{
	float NDF = normalDistributionFunction(microfacet);
	float n_v = dot(wo, normal);
	float n_l = dot(wi, normal);
	float reflectance = fresnelTerm(wi, microfacet) * geometryTerm(wi, wo, microfacet) * NDF / (4 * n_v * n_l);

	return vec2(reflectance, NDF);
}

float attenuation(vec3 position1, vec3 position2)
{
	float len = length(position1 - position2);
	return attenuation(len);
}

float pdfGGX(float NDF, vec3 microfacet)
{
	float cos = dot(normal, microfacet);
	float sin = 1 - cos * cos;
	return max(NDF * sin * cos, 0.000001);
}

vec3 lightSpecular(vec3 position, vec3 normal, vec3 wi, vec3 wo, int k)
{
	vec3 microfacet = normalize(wo + wi);

	// in the brdf both rays need to be pointing away from the surface
	vec2 res = brdf(wo, wi, microfacet);
	float reflectance = res.x;
	float NDF = res.y;

	return reflectance * material_specular_color * lightColors[k] * dot(normal, wi) * lightIntensities[k] / attenuation(lightPositions[k], position);
}

vec3 lightDiffuse(vec3 position, vec3 normal, vec3 wi, int k)
{
	return material_diffuse_color * OneOverPI * lightColors[k] * dot(normal, wi) * lightIntensities[k] / attenuation(lightPositions[k], position);
}

vec3 get_point_lights()
{
	vec3 contributions = vec3(0, 0, 0);
	for (int i = 0; i < lightsCount; ++i)
	{
		vec3 wi = normalize(lightPositions[i] - position.xyz);

		contributions += lightDiffuse(position.xyz, normal, wi, i);
		contributions += lightSpecular(position.xyz, normal, wi, wo, i);
	}
	return contributions;
}

vec3 get_environment_map(int samples)
{
	vec3 contributions = vec3(0, 0, 0);
	for (int i = 0; i < samples; ++i)
	{
		vec3 half_vector = halfvector_GGX_sample();
		vec3 microfacet = normalize(TBN * half_vector);

		vec3 wi = reflect(-wo, microfacet);
		
		vec2 res = brdf(wo, wi, microfacet);
		float reflectance = res.x;
		float NDF = res.y;
		vec4 tex = texture(environment_map, ray_to_uv(wi));
		vec3 tex_color = tex.rgb / tex.a;

		float pdf = pdfGGX(NDF, microfacet);
		
		contributions += reflectance * material_specular_color *  tex_color * dot(normal, wi) / pdf;
	}
	// Average all the contributions
	return contributions / samples;
}

vec3 getContributions()
{
	return get_point_lights() + get_environment_map(100);
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
	fragColor = vec4(getContributions(), 1);
	//fragColor = vec4(lightsCount, lightsCount, lightsCount, 1);
}