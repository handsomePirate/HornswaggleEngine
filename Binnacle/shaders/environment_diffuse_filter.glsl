#version 330 core

in vec2 uv;
in vec4 position;

uniform sampler2D original;
uniform int original_width;
uniform int original_height;

layout(location = 0) out vec4 fragColor;
const float PI = 3.14159265358979323846;

vec3 uv_to_ray(vec2 uv)
{
	float phi = uv.x * 2 * PI;
	float theta = uv.y * PI;
	
	float x = sin(theta) * cos(phi);
	float y = cos(theta);
	float z = sin(theta) * sin(phi);
	
	return vec3(x, y, z);
}

void main(void)
{
	vec3 N = uv_to_ray(uv);

	vec3 sum = vec3(0, 0, 0);
	float dot_sum = 0;;

	//fragColor = vec4(texture(original, uv).rgb / texture(original, uv).a, 1);
	//return;
	for (int j = 0; j < original_height; ++j)
	{
		for (int i = 0; i < original_width; ++i)
		{
			vec2 original_uv = vec2(i / float(original_width), j / float(original_height));
			vec3 L = uv_to_ray(original_uv);
			vec4 tex = texture(original, original_uv);
			vec3 I = tex.rgb / tex.a;

			float dot = max(0, dot(L, N));
			sum += dot * I;
			dot_sum += dot;
		}
	}
	fragColor = vec4(sum / dot_sum, 1);
}