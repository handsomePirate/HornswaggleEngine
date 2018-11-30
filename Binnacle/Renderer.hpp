#pragma once
#include <memory>

#include <GL/glew.h>

#include "Scene.hpp"
#include "Shader.hpp"

enum class buffer_attrib
{
	VERTEX,
	POINT_LIGHT
};

enum class vizualization
{
	LIGHTS = 0,
	ENVIRONMENT_MAP = 1
};

// The object that takes care of rasterazing the scene
struct renderer
{
	renderer();
	renderer(const renderer& rm) = delete;
	renderer& operator=(const renderer& rm) = delete;
	renderer(renderer && rm) = default;
	renderer& operator=(renderer && rm) = default;
	virtual ~renderer() = default;

	void render(const std::shared_ptr<scene>& scn_ptr, 
		const std::shared_ptr<std::map<int, material>>& mat_ptr, 
		const std::shared_ptr<environment>& env_ptr) const;

	void enable(const vizualization& option) const;
	void disable(const vizualization& option) const;

	bool is_enabled(const vizualization& option) const;

	void set_buffer_attrib(const buffer_attrib& option) const;
	void set_background_color(const glm::vec3& color);

private:
	GLuint vbo_{};
	GLuint vao_{};
	GLuint vio_{};

	bool *vizualization_options_;
	glm::vec3 background_color_;
};
