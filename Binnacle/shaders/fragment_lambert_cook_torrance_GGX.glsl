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

vec3 material_color;
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

layout(location = 0) out vec4 fragColor;

const float PI = 3.14159265358979323846;

//==========================REMOVE OR REPLACE==============================
#define FLT_MAX 3.402823466e+38

void seed(int s)
{
	//index = ((s ^ (s & 0x0000000f << 20)) >> 3) * s ^ floatBitsToInt(gl_FragCoord.z);
	index = s ^ floatBitsToInt(gl_FragCoord.z);
}

float rand()
{
	index = abs(468 * index + 11251) % 1654897; // 1654897
    return index / 1654897.0;
}

vec3 sample_uniform()
{
	float r1 = rand();
	float r2 = rand();

	float r = sqrt(r1);
    float phi = 2 * PI * r2;
 
    return vec3(cos(phi) * r, sqrt(1 - r1), sin(phi) * r);
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
	return contributions * material.color / PI;
}

vec3 environment_map_lambert(int samples)
{
	// For now use sampling => uniformly sample over the hemisphere
	vec3 contributions = vec3(0, 0, 0);
	for (int i = 0; i < samples; ++i)
	{
		vec3 tex_color = texture(environment_map_diffuse, ray_to_uv(normal)).rgb;
		
		contributions += tex_color;
	}
	// Average all the contributions
	return contributions / samples * material.color;
}

vec3 diffuse_brdf()
{
	vec3 sum = point_lights_lambert();
	sum += environment_map_lambert(20);
	return sum;
}

void prepare_surface_data()
{
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

	const int mult_constant = 100;
	int s = int(gl_FragCoord.y * mult_constant) * windowWidth * mult_constant + int(gl_FragCoord.x * mult_constant);
	seed(s);
}

void main(void)
{
	prepare_surface_data();
	
	vec3 wo = normalize(camera - position.xyz);
	vec3 diffuse_color = diffuse_brdf();
	fragColor = vec4(diffuse_color, 1);
}