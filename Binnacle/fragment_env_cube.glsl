#version 400 core // 330

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

//uniform samplerCube cubemap;
uniform sampler2D environment_map;

uniform vec3 lightPositions[20]; // max number of lights in the scene
uniform vec3 lightColors[20];
uniform float lightIntensities[20];

uniform int lightsCount;

uniform vec3 camera;

uniform bool useTexture;
uniform vec3 matColor;

uniform Material material;

const float PI = 3.14159265358979323846;

//out vec4 fragColor;

layout(location = 0) out vec4 fragColor;

void main(void)
{
	vec3 direction = normalize(-norm);
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
	vec2 map_coords = vec2(map_x, map_y);

	fragColor = vec4(texture(environment_map, map_coords).rgb, 1);

	//fragColor = vec4(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi), 1);
	//fragColor = vec4(map_x < 0.0001, map_x < 0.0002, map_x < 0.0003, 1);
}