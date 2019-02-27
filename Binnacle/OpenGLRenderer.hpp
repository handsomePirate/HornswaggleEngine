#pragma once

#include <memory>

#include <GL/glew.h>

#include "Scene.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"

struct opengl_renderer : renderer
{
	explicit opengl_renderer(GLuint texture_draw_program);
	opengl_renderer(const opengl_renderer& rm) = delete;
	opengl_renderer& operator=(const opengl_renderer& rm) = delete;
	opengl_renderer(opengl_renderer && rm) = delete;
	opengl_renderer& operator=(opengl_renderer && rm) = delete;
	~opengl_renderer();

	void render(const std::shared_ptr<scene>& scn_ptr,
		const std::shared_ptr<std::map<int, material>>& mat_ptr,
		const std::shared_ptr<environment>& env_ptr) const override;

	void enable(const vizualization& option) const override;
	void disable(const vizualization& option) const override;

	bool is_enabled(const vizualization& option) const override;

	void set_background_color(const glm::vec3& color) override;

	void change_viewport_size(unsigned int width, unsigned int height) override;
private:
	void set_buffer_attrib(const buffer_attrib& option) const;

	GLuint vbo_{};
	GLuint vio_{};

	bool *vizualization_options_;
	glm::vec3 background_color_{};
};