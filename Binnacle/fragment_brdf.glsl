#version 330 core
#extension GL_NV_shadow_samplers_cube: enable

in vec4 position;
flat in vec3 norm;
varying in vec3 varNorm;
in vec3 color;
in vec2 coords;

uniform sampler2D texSampler;
uniform samplerCube cubemap;

uniform vec3 lightPositions[20]; // max number of lights in the scene
uniform vec3 lightColors[20];
uniform float lightDiffI[20];
uniform float lightSpecI[20];

uniform int lightsCount;

uniform vec3 camera;

uniform bool useTexture;
uniform vec3 matColor;

out vec4 fragColor;

void main(void)
{
	//fragColor = vec4(abs(varNorm), 1);
	//return;
	vec4 diff_color = vec4(textureCube(cubemap, reflect(position.xyz - camera, varNorm)));
	fragColor = clamp(diff_color, 0, 1);

	const int N = 10;
	vec3 color = vec3(0, 0, 0);

	for (int i = 0; i < N; ++i)
	{
		
	}
}