#pragma once
#include <memory>
#include <string>
#include <map>
#include <queue>
#include <chrono>

#include "Renderer.hpp"
#include "Scene.hpp"
#include "Shader.hpp"

#include <GLFW/glfw3.h>

typedef unsigned int rm_choice;
#define BINNACLE (unsigned int)1

// Serves the purpose of maintaining the renderer to use, the scene to render and the window to render to
// TODO: make it possible to render to texture
// TODO: monitor window size changes
struct render_manager
{
	render_manager(bool fullscreen, int samples, int major_version, int minor_version, int width, int height, const std::string& window_text);
	render_manager(const render_manager& rm) = delete;
	render_manager& operator=(const render_manager& rm) = delete;
	render_manager(render_manager && rm) = default;
	render_manager& operator=(render_manager && rm) = default;
	virtual ~render_manager();

	bool is_valid() const;

	float get_aspect_ratio() const;

	int create_shader_program(const std::string& vertex_shader_file, const std::string& fragment_shader_file, const std::string& geometry_shader_file = "") const;
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

	void load_model(const std::string& filename_model, int mat_id = -1) const;
	int create_material(GLuint program, const std::string& filename_texture = "", bool smooth = false);
	int create_material(GLuint program, const glm::vec3& color, bool smooth = false);
	void delete_material(int index);

	float get_fps();

	void update();
	void render() const;

	bool should_end() const;
private:
	virtual void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
	virtual void mouse_move_callback(GLFWwindow *window, double x_pos, double y_pos);
	virtual void window_resize_callback(GLFWwindow *window, int width, int height);

	std::unique_ptr<renderer> rnd_ptr_;
	std::shared_ptr<scene> scn_ptr_;
	std::shared_ptr<environment> env_ptr_;

	GLFWwindow *window_; // TODO: needs init and destruct
	bool valid_;

	bool should_end_;

	float width_;
	float height_;

	std::shared_ptr<std::map<int, material>> mat_ptr_;
	std::queue<int> free_ids_;
	int next_free_id_ = 0;

	std::shared_ptr<std::map<GLuint, shader_program>> shd_ptr_;

	std::unique_ptr<std::map<int, int>> clb_ptr_;
	std::map<int, bool> keys_manager_;

	double cursor_dx_{};
	double cursor_dy_{};

	double cursor_x_{};
	double cursor_y_{};

	bool reset_cursor_ = true;
	bool reset_camera_ = true;

	std::chrono::high_resolution_clock::time_point frame_start_;
	unsigned long frame_count_ = 0;

	bool first_update_ = true;
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
