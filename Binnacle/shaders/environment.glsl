const float PI = 3.14159265358979323846;

vec2 ray_to_uv(vec3 direction)
{
	float cos_theta = direction.y;
	float theta = acos(cos_theta);

	float tan_phi = direction.z / direction.x;
	float phi = atan(tan_phi);
	
	if (direction.x < 0)
		phi += PI;

	float map_x = (phi) / (2 * PI);
	float map_y = theta / PI;

	return vec2(map_x, map_y);
}

vec3 uv_to_ray(vec2 uv)
{
	float phi = uv.x * 2 * PI;
	float theta = uv.y * PI;
	
	float x = sin(theta) * cos(phi);
	float y = cos(theta);
	float z = sin(theta) * sin(phi);
	
	return vec3(x, y, z);
}