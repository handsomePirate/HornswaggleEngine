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
	explicit renderer(GLuint texture_draw_program);
	renderer(const renderer& rm) = delete;
	renderer& operator=(const renderer& rm) = delete;
	renderer(renderer && rm) = delete;
	renderer& operator=(renderer && rm) = delete;
	virtual ~renderer() = default;

	virtual void render(const std::shared_ptr<scene>& scn_ptr, 
		const std::shared_ptr<std::map<int, material>>& mat_ptr, 
		const std::shared_ptr<environment>& env_ptr) const = 0;

	virtual void enable(const vizualization& option) const = 0;
	virtual void disable(const vizualization& option) const = 0;

	virtual bool is_enabled(const vizualization& option) const = 0;

	virtual void set_background_color(const glm::vec3& color) = 0;

	virtual void change_viewport_size(unsigned int width, unsigned int height) = 0;

	void render_texture(GLuint tex) const;
private:
	GLuint texture_draw_program_;
	GLuint quad_vbo_{};
};
