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
#include "PathManipulator.hpp"

#define ENABLE_DEBUG_CONTEXT
#undef ENABLE_DEBUG_CONTEXT

// Serves the purpose of maintaining the renderer to use, the scene to render and the window to render to
struct render_manager
{
	render_manager();
	render_manager(bool fullscreen, int samples, int major_version, int minor_version, int width, int height, const std::string& window_text, std::string& root_path, bool window_visible = true);
	render_manager(const render_manager& rm) = delete;
	render_manager& operator=(const render_manager& rm) = delete;
	render_manager(render_manager && rm) = default;
	render_manager& operator=(render_manager && rm) = default;
	virtual ~render_manager();

	// Has been properly initialized?
	bool is_valid() const;

	float get_aspect_ratio() const;

	// Assemble the shader program to use from source files
	int create_shader_program(const std::string& vertex_shader_file, const std::string& fragment_shader_file, const std::string& geometry_shader_file = "") const;
	int create_compute_shader(const std::string& compute_shader_file) const;
	void init_opengl_renderer();
	void init_path_tracer();

	// Append/Clear&Append new light sources
	template<class ...T>
	void add_lights(T&&... args);
	template<class ...T>
	void set_lights(T&&... args);

	void renderer_enable(const vizualization& option) const;
	void renderer_disable(const vizualization& option) const;
	bool renderer_is_enabled(const vizualization& option) const;

	// Prepare the renderer to render to a texture
	void init_texture_rendering(int width, int height);
	unsigned char *render_to_texture(bool screen = false);

	GLuint filter(const std::string& fragment, GLuint tex, unsigned int in_width, unsigned int in_height, unsigned int out_width, unsigned int out_height) const;

	void set_camera(glm::vec3&& position, glm::vec3&& focus, glm::vec3&& up, float fov, float aspect, float z_near, float z_far, bool perspective = true) const;
	camera& get_camera() const;

	void set_background_color(const glm::vec3& color) const;

	// Load an OBJ model from a file (returns its assigned id)
	int load_model(const std::string& filename_model, bool smooth, int mat_id = -1);
	int load_model_data(const std::vector<vertex>& vertices, const std::vector<unsigned int>& indices, int mat_id = -1);
	int load_model_data(float *vertex_positions, size_t vertex_count, unsigned int* indices, size_t index_count, int mat_id = -1);
	int load_model_data(float *vertex_positions, float *normals, size_t vertex_count, unsigned int* indices, size_t index_count, int mat_id = -1);
	// Delete a model identified by its id
	void delete_model(int index);

	void register_path(std::string& name, std::string& path_from_root) const;
	void register_path(std::string&& name, std::string& path_from_root) const;
	void register_path(std::string& name, std::string&& path_from_root) const;
	void register_path(std::string&& name, std::string&& path_from_root) const;
	std::string get_full_path(const std::string& name) const;

	// Create an instance of a model identified by its id
	int instance_model(int index);
	// Create an instance of a model identified by the instance id
	void delete_model_instance(int index);

	// Get a handle of a model instance to be able to transform it
	instance_handle get_instance_handle(int index) const;

	// Create a textured material (get its id)
	int create_material(GLuint program, const std::string& filename_texture = "", const std::string& filename_normals = "");
	// Create a colored material (get its id)
	int create_material(GLuint program, const glm::vec3& color, const std::string& filename_normals = "");
	int create_material(GLuint program, float r, float g, float b, const std::string& filename_normals = "");
	// Delete a material by its id
	void delete_material(int index);

	// Load an environment cube map from six images
	void load_environment_cube_map(const std::string& neg_z, const std::string& pos_z,
		const std::string& neg_x, const std::string& pos_x,
		const std::string& neg_y, const std::string& pos_y) const;

	void load_hdr_environment(const std::string& hdr_img_file, int width, int height, const std::string& ldr_diffuse_file = "") const;

	// Press the timer
	void start_framerate();

	// Update matrices etc., alternates with "render"
	float update();
	// Render all the loaded model instances
	void render() const;

	// OpenGL needs to terminate the window or the user requests it
	bool should_end() const;
private:
	void create_paths(std::string& root_path);

	template<class ...T>
	void init_environment(camera&& cam, GLuint cube_program, GLuint lights_program, T&&... args);
	template<class ...T>
	void init_environment(GLuint cube_program, GLuint lights_program, T&&... args);

	float get_fps();

	virtual void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
	virtual void mouse_move_callback(GLFWwindow *window, double x_pos, double y_pos);
	virtual void window_resize_callback(GLFWwindow *window, int width, int height);

	float get_seconds(const std::chrono::high_resolution_clock::time_point& frame_start) const;

	static void reorder_pixels(rgba *pixels, size_t width, size_t height);

	GLFWwindow *window_; // TODO: needs init and destruct

	std::unique_ptr<renderer> rnd_ptr_;
	std::shared_ptr<scene> scn_ptr_;
	std::shared_ptr<environment> env_ptr_;

	bool valid_;

	bool should_end_;

	int width_;
	int height_;

	std::unique_ptr<std::map<int, std::vector<int>>> ins_ptr_;

	std::unique_ptr<std::map<int, model>> mod_ptr_;
	std::queue<int> mod_free_ids_;
	int mod_next_free_id_ = 0;

	std::shared_ptr<std::map<int, material>> mat_ptr_;
	std::queue<int> mat_free_ids_;
	int mat_next_free_id_ = 0;

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

	std::chrono::high_resolution_clock::time_point last_update_;

	unsigned int vertex_count_ = 0;
	unsigned int poly_count_ = 0;

	std::unique_ptr<path_manipulator> path_manipulator_;

	// texture rendering variables
	GLuint texture_fbo_ = 0;
	GLuint rendered_texture_{};
	GLuint depth_render_buffer_{};

	GLuint quad_shader_program_{};

	int texture_width_{};
	int texture_height_{};

	bool window_visible_;
};

template <class ... T>
void render_manager::init_environment(camera&& cam, GLuint cube_program, GLuint lights_program, T&&... args)
{
	env_ptr_ = std::make_shared<environment>(cam, cube_program, lights_program);
	env_ptr_->set_lights(std::forward<T>(args)...);
}

template <class ... T>
void render_manager::init_environment(GLuint cube_program, GLuint lights_program, T&&... args)
{
	env_ptr_ = std::make_shared<environment>(camera(), cube_program, lights_program);
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