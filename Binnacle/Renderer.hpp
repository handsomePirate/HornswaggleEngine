#pragma once
#include <memory>

#include <GL/glew.h>

#include "Scene.hpp"
#include "Shader.hpp"

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
private:
	GLuint vbo_{};
	GLuint vao_{};
	GLuint vio_{};
};
