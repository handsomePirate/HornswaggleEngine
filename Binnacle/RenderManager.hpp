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
// TODO: monitor window size changes
struct render_manager
{
	render_manager(bool fullscreen, int samples, int major_version, int minor_version, int width, int height, const std::string& window_text, GLFWkeyfun key_callback);
	render_manager(const render_manager& rm) = delete;
	render_manager& operator=(const render_manager& rm) = delete;
	render_manager(render_manager && rm) = default;
	render_manager& operator=(render_manager && rm) = default;
	virtual ~render_manager();

	bool is_valid() const;

	float get_aspect_ratio() const;

	void select_renderer(rm_choice rmc);
	void select_scene(const std::shared_ptr<scene>& scn_ptr);
	template<class ...T>
	void set_environment(camera&& cam, T&&... args);
	template<class ...T>
	void add_lights(T&&... args);
	template<class ...T>
	void set_lights(T&&... args);

	void change_camera(glm::vec3&& position, glm::vec3&& focus, glm::vec3&& up, float fov, float aspect, float z_near, float z_far) const;
	camera& get_camera() const;

	void load_model(const std::string& filename_model, const std::string& filename_texture = "") const;

	void update() const;
	void render() const;

private:
	std::unique_ptr<renderer> rnd_ptr_;
	std::shared_ptr<scene> scn_ptr_;
	std::shared_ptr<environment> env_ptr_;

	GLFWwindow *window_; // TODO: needs init and destruct
	bool valid_;

	float width_;
	float height_;
};


template <class ... T>
void render_manager::set_environment(camera&& cam, T&&... args)
{
	env_ptr_ = std::make_shared<environment>(cam);
	env_ptr_->set_lights(std::forward<T>(args)...);
}

template <class ... T>
void render_manager::add_lights(T&&... args)
{
	if (env_ptr_)
		env_ptr_->add_lights(std::forward<T>(args)...);
}

template <class ... T>
void render_manager::set_lights(T&&... args)
{
	if (env_ptr_)
		env_ptr_->set_lights(std::forward<T>(args)...);
}
