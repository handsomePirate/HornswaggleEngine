#pragma once
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include "Texture.hpp"
#include "Scene.hpp"
#include <functional>

enum shader_type
{
	VERTEX,
	FRAGMENT,
	GEOMETRY,
	// and others
};

struct shader
{
	GLint text_length;
	std::string text;

	shader();

	shader(GLint text_length, std::string text);
};

struct shader_program
{
	shader_program();
	bool load_shader(shader_type&& type, const std::string& filename);
	bool compile_and_link_shaders();

	GLuint get_id() const;

	void update(const std::shared_ptr<environment>& env_ptr) const;
private:
	std::unordered_map<shader_type, std::string> shaders_;
	GLuint program_;

	std::vector<int> mat_ids_;
};

struct material
{
	material();
	explicit material(GLuint program);
	material(const std::string& filename_texture, GLuint program);
	material(const std::string& filename_texture, const std::string& filename_normals, GLuint program);
	material(const glm::vec3& color, GLuint program);
	material(const glm::vec3& color, const std::string& filename_normals, GLuint program);

	material(const material& rm) = delete;
	material& operator=(const material& rm) = delete;
	material(material && rm) = default;
	material& operator=(material && rm) = default;
	virtual ~material() = default;

	void set_as_active() const;
	void use_program(GLuint program);

	virtual void update();
protected:
	texture texture_;
	texture normal_map_;
	bool use_texture_;
	glm::vec3 color_;
	GLuint program_;

	bool initialized_;
};