#include <iostream>

#include <GL/glew.h>
#include <SOIL/SOIL.h>

#include "RenderManager.hpp"

render_manager::render_manager(const bool fullscreen, const int samples, const int major_version, const int minor_version, const int width, const int height, const std::string& window_text)
	: valid_(true), should_end_(false), width_(width), height_(height)
{
	// Initialize GLFW
	if (!glfwInit()) // TODO: probably move outside of the manager
	{
		// TODO: exchange for a logger
		std::cout << "Error: GLFW could not initialize!" << std::endl;
		valid_ = false;
		return;
	}

	// Fullfil the requirements
	glfwWindowHint(GLFW_SAMPLES, samples);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	// Create the window
	GLFWmonitor *monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;
	window_ = glfwCreateWindow(width, height, "OpenGL", monitor, nullptr);

	if (!window_)
	{
		// TODO: exchange for a logger
		std::cout << "Error: GLFW could not create a window!" << std::endl;
		glfwTerminate();
		valid_ = false;
		return;
	}

	// TODO: make sure that more applications at the same time do not break this
	glfwMakeContextCurrent(window_);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	const auto glew_err = glewInit();
	if (glew_err != GLEW_OK)
	{
		// TODO: exchange for a logger
		std::cout << "Error: GLEW could not initialize!" << std::endl;
		glfwTerminate();
		valid_ = false;
		window_ = nullptr;
		return;
	}

	// Get string info
	const auto renderer_str = glGetString(GL_RENDERER);
	const auto version_str = glGetString(GL_VERSION);

	// TODO: exchange for a logger
	std::cout << "Active renderer: " << renderer_str << std::endl;
	std::cout << "OpenGL version: " << version_str << std::endl;

	glfwSetInputMode(window_, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetWindowUserPointer(window_, this);

	const auto key_func = [](GLFWwindow *window, const int key, const int scancode, const int action, const int mods)
	{
		static_cast<render_manager *const>(glfwGetWindowUserPointer(window))->key_callback(window, key, scancode, action, mods);
	};

	const auto mouse_func = [](GLFWwindow *window, const double xpos, const double ypos)
	{
		static_cast<render_manager *const>(glfwGetWindowUserPointer(window))->mouse_move_callback(window, xpos, ypos);
	};

	const auto resize_func = [](GLFWwindow *window, const int width, const int height)
	{
		static_cast<render_manager *const>(glfwGetWindowUserPointer(window))->window_resize_callback(window, width, height);
	};

	glfwSetKeyCallback(window_, key_func);
	glfwSetCursorPosCallback(window_, mouse_func);
	glfwSetWindowSizeCallback(window_, resize_func);

	glfwGetCursorPos(window_, &cursor_x_, &cursor_y_);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glfwShowWindow(window_);

	mat_ptr_ = std::make_shared<std::map<int, material>>();
	shd_ptr_ = std::make_shared<std::map<GLuint, shader_program>>();
	clb_ptr_ = std::make_unique<std::map<int, int>>();
}

bool render_manager::is_valid() const
{
	return valid_;
}

float render_manager::get_aspect_ratio() const
{
	return width_ / height_;
}

int render_manager::create_shader_program(const std::string& vertex_shader_file,
	const std::string& fragment_shader_file, const std::string& geometry_shader_file) const
{
	auto shp = shader_program();

	shp.load_shader(VERTEX, vertex_shader_file);
	shp.load_shader(FRAGMENT, fragment_shader_file);
	if (!geometry_shader_file.empty())
		shp.load_shader(GEOMETRY, geometry_shader_file);

	if (!shp.compile_and_link_shaders())
	{
		// TODO: exchange for a logger
		std::cout << "Error: shader problem!" << std::endl;
	}
	else
	{
		std::cout << "Shaders done!" << std::endl;
	}

	(*shd_ptr_)[shp.get_id()] = shp;

	return shp.get_id();
}

void render_manager::select_renderer(const rm_choice rmc)
{
	if (valid_)
	{
		if (rmc == BINNACLE)
		{
			rnd_ptr_ = std::make_unique<renderer>();
		}
		else
		{
			// TODO: exchange for a logger
			std::cout << "Error: not an existing renderer!" << std::endl;
		}
	}
}

void render_manager::select_scene(const std::shared_ptr<scene>& scn_ptr)
{
	if (valid_)
		scn_ptr_ = scn_ptr;
}

void render_manager::change_camera(glm::vec3&& position, glm::vec3&& focus, glm::vec3&& up, const float fov, const float aspect, const float z_near, const float z_far) const
{
	auto& cam = env_ptr_->get_camera();

	cam.set_transform_focus(position, focus, up);
	cam.set_frustum(fov, aspect, z_near, z_far);
}

camera& render_manager::get_camera() const
{
	return env_ptr_->get_camera();
}

void render_manager::load_model(const std::string& filename_model, const bool smooth, const int mat_id)
{
	vertex_count_ += scn_ptr_->load_model(filename_model, smooth, mat_id);
}

#define ASSIGN_FREE_ID(free_vec, next_free)\
	int id = -1;\
	if (free_ids_.empty())\
		{id = next_free_id_++;}\
	else {id = free_ids_.front(); free_ids_.pop();}

int render_manager::create_material(const GLuint program, const std::string& filename_texture, const bool smooth)
{
	ASSIGN_FREE_ID(free_ids_, next_free_id_);
	(*mat_ptr_)[id] = material(filename_texture, program);
	
	scn_ptr_->use_material(id);

	return id;
}

int render_manager::create_material(const GLuint program, const glm::vec3& color)
{
	ASSIGN_FREE_ID(free_ids_, next_free_id_);
	(*mat_ptr_)[id] = material(color, program);

	scn_ptr_->use_material(id);

	return id;
}

void render_manager::delete_material(const int index)
{
	if (mat_ptr_->find(index) != mat_ptr_->end())
	{
		free_ids_.push(index);
		mat_ptr_->erase(index);
	}
}

void render_manager::load_environment_cube_map(const std::string& neg_z, const std::string& pos_z,
	const std::string& neg_x, const std::string& pos_x, const std::string& neg_y, const std::string& pos_y) const
{
	if (!env_ptr_)
		return;

	GLuint tex_id;
	glGenTextures(1, &tex_id);

	glActiveTexture(GL_TEXTURE31);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

	int width, height;
	unsigned char *image = SOIL_load_image(pos_x.c_str(), &width, &height, nullptr, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	image = SOIL_load_image(neg_x.c_str(), &width, &height, nullptr, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	image = SOIL_load_image(pos_z.c_str(), &width, &height, nullptr, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	image = SOIL_load_image(neg_z.c_str(), &width, &height, nullptr, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	image = SOIL_load_image(pos_y.c_str(), &width, &height, nullptr, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	image = SOIL_load_image(neg_y.c_str(), &width, &height, nullptr, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	env_ptr_->set_environment_map(tex_id);
}

#define time_now std::chrono::high_resolution_clock::now()

float render_manager::get_fps()
{
	const auto result = frame_count_ / get_seconds(frame_start_);
	frame_start_ = time_now;
	frame_count_ = 0;
	return result;
}

void render_manager::update()
{
	if (first_update_)
	{
		frame_start_ = time_now;
		frame_count_ = 0;
		first_update_ = false;
	}

	if (glfwWindowShouldClose(window_))
		should_end_ = true;

	if (valid_ && scn_ptr_ && env_ptr_)
	{
		scn_ptr_->update();

		get_camera().set_aspect(width_ / static_cast<float>(height_));

		const auto time_elapsed = get_seconds(last_update_);
		last_update_ = time_now;
		++frame_count_;
		const float speed = 5.0f * time_elapsed;

		if (keys_manager_[GLFW_KEY_A] || keys_manager_[GLFW_KEY_LEFT])
			get_camera().translate_local_2_d(speed, 0.0f, 0.0f);
		if (keys_manager_[GLFW_KEY_D] || keys_manager_[GLFW_KEY_RIGHT])
			get_camera().translate_local_2_d(-speed, 0.0f, 0.0f);
		if (keys_manager_[GLFW_KEY_W] || keys_manager_[GLFW_KEY_UP])
			get_camera().translate_local_2_d(0.0f, 0.0f, speed);
		if (keys_manager_[GLFW_KEY_S] || keys_manager_[GLFW_KEY_DOWN])
			get_camera().translate_local_2_d(0.0f, 0.0f, -speed);
		if (keys_manager_[GLFW_KEY_LEFT_SHIFT] || keys_manager_[GLFW_KEY_DOWN])
			get_camera().translate(0.0f, speed, 0.0f);
		if (keys_manager_[GLFW_KEY_LEFT_CONTROL] || keys_manager_[GLFW_KEY_DOWN])
			get_camera().translate(0.0f, -speed, 0.0f);

		if (reset_camera_)
		{
			get_camera().set_transform_forward(glm::vec3(0.0f, 0.5, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			reset_camera_ = false;
		}

		get_camera().create_matrices();
	}
	if (valid_ && rnd_ptr_ && env_ptr_ && GL_NO_ERROR == glGetError())
	{
		glfwPollEvents();

		for (auto && shd : *shd_ptr_)
			shd.second.update(env_ptr_);
	}
}


void render_manager::render() const
{
	if (valid_ && rnd_ptr_ && scn_ptr_ && GL_NO_ERROR == glGetError())
	{
		// glfwMakeContextCurrent(window_);
		rnd_ptr_->render(scn_ptr_, mat_ptr_);
		glfwSwapBuffers(window_);
	}
}

bool render_manager::should_end() const
{
	return should_end_;
}

void render_manager::key_callback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
{
	if (action == GLFW_PRESS)
		keys_manager_[key] = true;
	if (action == GLFW_RELEASE)
		keys_manager_[key] = false;

	if (keys_manager_[GLFW_KEY_X] && action == GLFW_PRESS)
	{
		reset_cursor_ = !reset_cursor_;
	}
	if (keys_manager_[GLFW_KEY_R] && action == GLFW_PRESS)
	{
		reset_camera_ = !reset_camera_;
	}
	if (keys_manager_[GLFW_KEY_I] && action == GLFW_PRESS)
	{
		std::cout << "vertex count: " << vertex_count_ << std::endl;
		std::cout << "FPS: " << get_fps() << std::endl;
	}

	if (key == GLFW_KEY_ESCAPE)
		should_end_ = true;
}

void render_manager::mouse_move_callback(GLFWwindow* window, const double x_pos, const double y_pos)
{
	const auto resize_const = 500.0f;

	cursor_dx_ = - (x_pos - cursor_x_) / resize_const;
	cursor_dy_ = (y_pos - cursor_y_) / resize_const;

	get_camera().rotate(glm::vec3(0.0f, 1.0f, 0.0f), cursor_dx_);
	get_camera().rotate_local(glm::vec3(1.0f, 0.0f, 0.0f), cursor_dy_);

	if (reset_cursor_)
	{
		cursor_x_ = width_ / 2.0f;
		cursor_y_ = height_ / 2.0f;
		glfwSetCursorPos(window, cursor_x_, cursor_y_);
	}
	else
	{
		cursor_x_ = x_pos;
		cursor_y_ = y_pos;
	}
}

void render_manager::window_resize_callback(GLFWwindow* window, const int width, const int height)
{
	width_ = width;
	height_ = height;

	glViewport(0, 0, width, height);
}

float render_manager::get_seconds(const std::chrono::high_resolution_clock::time_point& frame_start) const
{
	const auto frame_end = time_now;
	const auto duration = frame_end - frame_start;
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

	return milliseconds.count() * 0.001f;
}

render_manager::~render_manager()
{
	if (valid_)
		glfwTerminate();
}
