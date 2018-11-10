#version 330 core

struct Material
{
  vec3 color;
};

in vec4 position;
in vec3 varNorm;
in vec3 color;
in vec2 coords;

uniform vec3 camera;

uniform Material material;

layout(location = 0) out vec4 fragColor;

void main(void)
{
	vec3 normal;
	normal = normalize(varNorm);

	vec3 view = normalize(camera - position.xyz);
	float k = 0.2;
	fragColor = vec4(k * material.color + (1 - k) * material.color * pow(dot(view.xz, normal.xz), 4), 1.0);
}