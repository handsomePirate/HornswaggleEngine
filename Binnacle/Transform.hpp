#pragma once
#include <glm/glm.hpp>

static struct transform
{
	static glm::quat rotate(const glm::vec3& axis, float angle);
	static glm::quat rotate_local(float x, float y, float z, float angle, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);
	static glm::vec3 translate_local(float dx, float dy, float dz, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);
	static glm::vec3 translate_local_2_d(float dx, float dy, float dz, int axis, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);
	static glm::mat4 get_local_to_global_matrix(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);
	static glm::vec3 transform_vector(const glm::vec3& vec, const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);
};