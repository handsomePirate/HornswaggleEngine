#include "Transform.hpp"
#include <glm/gtc/quaternion.hpp>

glm::quat transform::rotate(const glm::vec3& axis, float angle)
{
	return glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), angle, axis); 
}

glm::quat transform::rotate_local(float x, float y, float z, float angle, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
{
	const glm::vec3 new_axis = transform::get_local_to_global_matrix(position, forward, up) * glm::vec4(x, y, z, 1.0f);
	return rotate(new_axis, angle);
}

glm::vec3 transform::translate_local(float dx, float dy, float dz, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
{
	const glm::vec3 new_offset = get_local_to_global_matrix(position, forward, up) * glm::vec4(dx, dy, dz, 1.0f);
	return new_offset;
}

glm::vec3 transform::translate_local_2_d(float dx, float dy, float dz, int axis, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
{
	if (dx == 0 && dy == 0 && dz == 0 || axis < 0 || axis > 2)
		return glm::vec3();

	glm::vec3 new_offset = get_local_to_global_matrix(position, forward, up) * glm::vec4(dx, dy, dz, 1.0f);
	float *zero = &new_offset[0] + axis;
	*zero = 0.0f;
	new_offset = normalize(new_offset) * sqrt(dx * dx + dy * dy + dz * dz);

	return new_offset;
}

glm::mat4 transform::get_local_to_global_matrix(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
{
	const auto aside = cross(up, forward);

	const auto a11 = aside.x; const auto a12 = aside.y; const auto a13 = aside.z;
	const auto a21 = up.x; const auto a22 = up.y; const auto a23 = up.z;
	const auto a31 = forward.x; const auto a32 = forward.y; const auto a33 = forward.z;
	const auto a41 = position.x; const auto a42 = position.y; const auto a43 = position.z;

	return inverse(glm::mat4( // transposed
		a11, a21, a31, a41,
		a12, a22, a32, a42,
		a13, a23, a33, a43,
		0, 0, 0, 1
	));
}

glm::vec3 transform::transform_vector(const glm::vec3& vec, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
{
	return get_local_to_global_matrix(position, forward, up) * glm::vec4(vec, 1.0f);
}
