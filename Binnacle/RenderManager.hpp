#pragma once
#include <memory>
#include <string>

#include "Renderer.hpp"
#include "Scene.hpp"

#include <GLFW/glfw3.h>

typedef unsigned int rm_choice;
#define BINNACLE (unsigned int)1

// Serves the purpose of maintaining the renderer to use, the scene to render and the window to render to
// TODO: make it possible to render to texture
struct render_manager
{
	render_manager(bool fullscreen, int samples, int major_version, int minor_version, int width, int height, const std::string& window_text, GLFWkeyfun key_callback);
	render_manager(const render_manager& rm) = delete;
	render_manager& operator=(const render_manager& rm) = delete;
	render_manager(render_manager && rm) = default;
	render_manager& operator=(render_manager && rm) = default;
	virtual ~render_manager();

	bool is_valid() const;

	void select_renderer(rm_choice rmc);
	void select_scene(const std::shared_ptr<scene>& scn_ptr);

	void update() const;
	void render() const;

private:
	std::unique_ptr<renderer> rnd_ptr_;
	std::shared_ptr<scene> scn_ptr_;

	GLFWwindow *window_; // TODO: needs init and destruct
	bool valid;
};
