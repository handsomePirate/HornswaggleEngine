#pragma once
#include <memory>
#include <unordered_map>

#include <GL/glew.h>

#include "Scene.hpp"

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

struct renderer
{
	renderer();
	renderer(const renderer& rm) = delete;
	renderer& operator=(const renderer& rm) = delete;
	renderer(renderer && rm) = default;
	renderer& operator=(renderer && rm) = default;
	virtual ~renderer() = default;

	void render(std::shared_ptr<scene> scn_ptr);

	bool load_shader(shader_type&& type, const std::string& filename);
	bool compile_and_link_shaders();
private:
	std::unordered_map<shader_type, std::string> shaders_;
};
