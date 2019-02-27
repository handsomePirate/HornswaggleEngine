#pragma once
#include <glm/detail/type_vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Texture.hpp"

#define FRAMERATE_STEP 1 / 60.0f

struct model_instance;

// This structure is used to enable the users of the render manager to transform the model instances in use
struct instance_handle
{
	explicit instance_handle(model_instance *mi);

	// relative
	void rotate(const glm::vec3& axis, float angle) const;
	void rotate(float x, float y, float z, float angle) const;
	void translate(const glm::vec4& offset) const;
	void translate(float dx, float dy, float dz) const;
	void scale(const glm::vec3& ratio) const;
	void scale(float ratio) const;

	// absolute
	void assign_position(const glm::vec4& position) const;
	void assign_position(float x, float y, float z) const;
	void assign_orientation(const glm::quat& orientation) const;
	void assign_orientation(float x, float y, float z, float w) const;
	void assign_scale(const glm::vec3& scale) const;
	void assign_scale(float x, float y, float z) const;

	// remove the instance reference from the handle (used mainly, when the instance is deleted)
	void delete_instance_reference();

private:
	model_instance *mi_;
};

static GLuint filter_texture(GLuint texture, const std::string& fragment, unsigned int in_width, unsigned int in_height, unsigned int out_width, unsigned int out_height);
