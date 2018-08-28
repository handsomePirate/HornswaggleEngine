#include <iostream>

#include <GL/glew.h>

#include "RenderManager.hpp"

render_manager::render_manager(const bool fullscreen, const int samples, const int major_version, const int minor_version, const int width, const int height, const std::string& window_text, GLFWkeyfun key_callback)
	: valid_(true), width_(width), height_(height)
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

	// TODO: set a callback for key press detection
	glfwSetInputMode(window_, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetKeyCallback(window_, key_callback);

	glfwShowWindow(window_);
}

bool render_manager::is_valid() const
{
	return valid_;
}

float render_manager::get_aspect_ratio() const
{
	return width_ / height_;
}

void render_manager::select_renderer(const rm_choice rmc)
{
	if (valid_)
	{
		if (rmc == BINNACLE)
		{
			rnd_ptr_ = std::make_unique<renderer>();
			rnd_ptr_->load_shader(VERTEX, "vertex.glsl");
			rnd_ptr_->load_shader(FRAGMENT, "fragment.glsl");

			if (!rnd_ptr_->compile_and_link_shaders())
			{
				// TODO: exchange for a logger
				std::cout << "Error: shader problem!" << std::endl;
			}
			else
			{
				std::cout << "Shaders done!" << std::endl;
			}
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

void render_manager::update() const
{
	if (valid_ && GL_NO_ERROR == glGetError())
	{
		glfwPollEvents();
		rnd_ptr_->update(env_ptr_);
	}
}


void render_manager::render() const
{
	if (valid_ && rnd_ptr_ && scn_ptr_ && GL_NO_ERROR == glGetError())
	{
		// glfwMakeContextCurrent(window_);
		rnd_ptr_->render(scn_ptr_);
		glfwSwapBuffers(window_);
	}
}

render_manager::~render_manager()
{
	if (valid_)
		glfwTerminate();
}
