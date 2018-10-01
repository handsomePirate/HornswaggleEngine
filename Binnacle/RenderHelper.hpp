#pragma once
#include <glm/detail/type_vec3.hpp>
#include <glm/gtc/quaternion.hpp>

struct model_instance;

struct model_handle
{
	explicit model_handle(model_instance *mi);

	void rotate(const glm::vec3& axis, float angle) const;
	void rotate(float x, float y, float z, float angle) const;
	void translate(const glm::vec4& offset) const;
	void translate(float dx, float dy, float dz) const;
	void scale(const glm::vec3& ratio) const;
	void scale(float ratio) const;

	void assign_position(const glm::vec4& position) const;
	void assign_position(float x, float y, float z) const;
	void assign_orientation(const glm::quat& orientation) const;
	void assign_orientation(float x, float y, float z, float w) const;
	void assign_scale(const glm::vec3& scale) const;
	void assign_scale(float x, float y, float z) const;

	void delete_instance_reference();

private:
	model_instance *mi_;
};
