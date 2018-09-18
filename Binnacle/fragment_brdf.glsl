#version 330 core
#extension GL_NV_shadow_samplers_cube: enable

in vec4 position;
flat in vec3 norm;
in vec3 varNorm;
in vec3 color;
in vec2 coords;
in mat3 TBN;

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
uniform vec3 matColor;

//out vec4 fragColor;

layout(location = 0) out vec4 fragColor;

void main(void)
{
	//fragColor = vec4(abs(varNorm), 1);
	//return;

	vec3 normal = varNorm;

	if (useTexture)
	{
		float normal_strength = 1;

		normal = texture(normalMap, coords).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = mix(vec3(0, 0, 1), normal, normal_strength);

		normal = normalize(TBN * normal);
	}

	vec4 diff_color = vec4(textureCube(cubemap, reflect(position.xyz - camera, normal)));
	fragColor = clamp(diff_color, 0, 1);

	const int N = 10;
	vec3 color = vec3(0, 0, 0);

	for (int i = 0; i < N; ++i)
	{
		
	}
}