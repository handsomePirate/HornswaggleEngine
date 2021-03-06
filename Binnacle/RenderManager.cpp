#include <iostream>

#include <GL/glew.h>
#include <SOIL/SOIL.h>

#include "RenderManager.hpp"
#include "RenderHelper.hpp"
#include "OpenGLRenderer.hpp"
#include "GPUPathTracer.hpp"

render_manager::render_manager()
	: window_(nullptr), valid_(false), should_end_(false), width_(0), height_(0), window_visible_(false) {}

render_manager::render_manager(const bool fullscreen, int samples, const int major_version, const int minor_version, const int width, const int height, const std::string& window_text, std::string& root_path, const bool window_visible)
	: window_(nullptr), valid_(true), should_end_(false), width_(width), height_(height), window_visible_(window_visible)
{
	if (!window_visible)
		samples = 1;

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

	create_paths(root_path);

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
	glEnable(GL_PROGRAM_POINT_SIZE);
#ifdef ENABLE_DEBUG_CONTEXT
	glEnable(GL_DEBUG_OUTPUT);

	const auto error_func = [](int error, const char* description)
	{
		fprintf(stdout, "OpenGL error: \n%s\n", description);
		std::cin.get();
	};

	glfwSetErrorCallback(error_func);
#endif

	if (window_visible)
		glfwShowWindow(window_);

	mat_ptr_ = std::make_shared<std::map<int, material>>();
	shd_ptr_ = std::make_shared<std::map<GLuint, shader_program>>();
	clb_ptr_ = std::make_unique<std::map<int, int>>();

	if (!window_visible)
		init_texture_rendering(width, height);
}

bool render_manager::is_valid() const
{
	return valid_;
}

float render_manager::get_aspect_ratio() const
{
	return width_ / float(height_);
}

int render_manager::create_shader_program(const std::string& vertex_shader_file,
	const std::string& fragment_shader_file, const std::string& geometry_shader_file) const
{
	auto shp = shader_program();

	const std::string shaders_path = path_manipulator_->get_full_path("shaders");

	std::cout << "Shaders: " << vertex_shader_file << ", " << fragment_shader_file;
	if (!geometry_shader_file.empty())
		std::cout << ", " << geometry_shader_file;
	std::cout << std::endl;

	if (!shp.load_shader(VERTEX, shaders_path + vertex_shader_file))
		std::cout << "Cannot open or read from " << vertex_shader_file << "!" << std::endl;
	if (!shp.load_shader(FRAGMENT, shaders_path + fragment_shader_file))
		std::cout << "Cannot open or read from " << fragment_shader_file << "!" << std::endl;
	if (!geometry_shader_file.empty())
	{
		if (!shp.load_shader(GEOMETRY, shaders_path + geometry_shader_file))
			std::cout << "Cannot open or read from " << geometry_shader_file << "!" << std::endl;
	}

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

int render_manager::create_compute_shader(const std::string& compute_shader_file) const
{
	auto cmp_shd = shader_program();

	const std::string shaders_path = path_manipulator_->get_full_path("shaders");

	if (!cmp_shd.load_shader(COMPUTE, shaders_path + compute_shader_file))
		std::cout << "Cannot open or read from " << compute_shader_file << "!" << std::endl;
	if (!cmp_shd.compile_and_link_shaders(false))
	{
		// TODO: exchange for a logger
		std::cout << "Error: shader problem!" << std::endl;
	}

	(*shd_ptr_)[cmp_shd.get_id()] = cmp_shd;

	return cmp_shd.get_id();
}

void render_manager::init_opengl_renderer()
{
	// TODO: scratch last renderer instance (and the scene, ...)
	if (valid_)
	{
		// TODO: make a list of required shaders
		quad_shader_program_ = create_shader_program("quad_vertex.glsl", "quad_fragment.glsl", "quad_geometry.glsl");
		rnd_ptr_ = std::make_unique<opengl_renderer>(quad_shader_program_);
		mod_ptr_ = std::make_unique<std::map<int, model>>();
		ins_ptr_ = std::make_unique<std::map<int, std::vector<int>>>();
		scn_ptr_ = std::make_shared<scene>();
		init_environment(create_shader_program("vertex.glsl", "fragment_env_cube.glsl"), create_shader_program("vertex_lights.glsl", "fragment_lights.glsl"));
	}
}

void render_manager::init_path_tracer()
{
	if (valid_)
	{
		quad_shader_program_ = create_shader_program("quad_vertex.glsl", "quad_fragment.glsl", "quad_geometry.glsl");
		// TODO: destroy the following shader
		GLuint path_tracer_program = create_compute_shader("path_trace_compute.glsl");

		rnd_ptr_ = std::make_unique<gpu_path_tracer>(quad_shader_program_, path_tracer_program, width_, height_);
		mod_ptr_ = std::make_unique<std::map<int, model>>();
		ins_ptr_ = std::make_unique<std::map<int, std::vector<int>>>();
		scn_ptr_ = std::make_shared<scene>();
		env_ptr_ = std::make_shared<environment>(camera());
	}
}

void render_manager::renderer_enable(const vizualization& option) const
{
	rnd_ptr_->enable(option);
}

void render_manager::renderer_disable(const vizualization& option) const
{
	rnd_ptr_->disable(option);
}

bool render_manager::renderer_is_enabled(const vizualization& option) const
{
	return rnd_ptr_->is_enabled(option);
}

void render_manager::init_texture_rendering(const int width, const int height)
{
	//TODO: delete previous fbo, texture, etc.

	texture_width_ = width;
	texture_height_ = height;

	glGenFramebuffers(1, &texture_fbo_);
	glBindFramebuffer(GL_FRAMEBUFFER, texture_fbo_);

	// The texture we're going to render to
	glGenTextures(1, &rendered_texture_);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, rendered_texture_);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// The depth buffer
	glGenRenderbuffers(1, &depth_render_buffer_);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer_);

	// Set "rendered_texture_" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendered_texture_, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Render to texture error: Framebuffer not correctly set!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width_, height_);
}

unsigned char *render_manager::render_to_texture(const bool screen)
{
	if (screen)
		init_texture_rendering(width_, height_);
	glBindFramebuffer(GL_FRAMEBUFFER, texture_fbo_);
	glViewport(0, 0, texture_width_, texture_height_);

	rnd_ptr_->render(scn_ptr_, mat_ptr_, env_ptr_);

	const auto image_data_size = texture_width_ * texture_height_;
	const auto image_data = new rgba[image_data_size];
	glReadPixels(0, 0, texture_width_, texture_height_, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<unsigned char *>(&image_data[0]));
	
	reorder_pixels(&image_data[0], texture_width_, texture_height_);
	if (screen)
		SOIL_save_image("screen.bmp", SOIL_SAVE_TYPE_BMP, texture_width_, texture_height_, 4, reinterpret_cast<unsigned char *>(&image_data[0]));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width_, height_);

	return reinterpret_cast<unsigned char *>(image_data);
}

GLuint render_manager::filter(const std::string& fragment, const GLuint tex, const unsigned int in_width, const unsigned int in_height,
	const unsigned int out_width, const unsigned int out_height) const
{
	//const auto program = create_shader_program("filter_vertex.glsl", fragment);
	//const auto filtered = rnd_ptr_->filter(program, tex, in_width, in_height, out_width, out_height);
	////glDeleteProgram(program);
	//glViewport(0, 0, width_, height_);
	//return filtered;
	return 0;
}

void render_manager::set_camera(glm::vec3&& position, glm::vec3&& focus, glm::vec3&& up, const float fov, const float aspect, const float z_near, const float z_far, bool perspective) const
{
	if (env_ptr_)
	{
		auto cam = &env_ptr_->get_camera();

		cam->set_transform_focus(position, focus, up);
		cam->set_frustum(fov, aspect, z_near, z_far);
		cam->set_perspective(perspective);
	}
}

camera& render_manager::get_camera() const
{
	return env_ptr_->get_camera();
}

void render_manager::set_background_color(const glm::vec3& color) const
{
	rnd_ptr_->set_background_color(color);
}

int render_manager::load_model(const std::string& filename_model, const bool smooth, const int mat_id)
{
	ASSIGN_FREE_ID(mod_free_ids_, mod_next_free_id_);
	const std::string models_path = path_manipulator_->get_full_path("models");
	(*mod_ptr_)[id] = model(models_path + filename_model, smooth, mat_id);

	return id;
}

int render_manager::load_model_data(const std::vector<vertex>& vertices, const std::vector<unsigned int>& indices, const int mat_id)
{
	ASSIGN_FREE_ID(mod_free_ids_, mod_next_free_id_);
	(*mod_ptr_)[id] = model(vertices, indices, mat_id);

	return id;
}

int render_manager::load_model_data(float* vertex_positions, const size_t vertex_count, unsigned int* indices,
                                     const size_t index_count, const int mat_id)
{
	std::vector<vertex> vertices(vertex_count);
	std::vector<unsigned int> indices_vec(index_count);
	
	auto vert_pos = reinterpret_cast<glm::vec3 *>(vertex_positions);
	for (size_t i = 0; i < vertex_count; ++i)
	{
		vertices[i] = vertex(glm::vec4(vert_pos->x, vert_pos->y, vert_pos->z, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec2(0, 0));
		++vert_pos;
	}

	for (size_t i = 0; i < index_count; ++i)
	{
		indices_vec[i] = indices[i];
	}
	
	return load_model_data(vertices, indices_vec, mat_id);
}

int render_manager::load_model_data(float *vertex_positions, float *normals, size_t vertex_count, unsigned int * indices, size_t index_count, int mat_id)
{
	std::vector<vertex> vertices(vertex_count);
	std::vector<unsigned int> indices_vec(index_count);

	auto vert_pos = reinterpret_cast<glm::vec3 *>(vertex_positions);
	auto norm_pos = reinterpret_cast<glm::vec3 *>(normals);
	for (size_t i = 0; i < vertex_count; ++i)
	{
		vertices[i] = vertex(glm::vec4(vert_pos->x, vert_pos->y, vert_pos->z, 1), glm::vec3(norm_pos->x, norm_pos->y, norm_pos->z), glm::vec3(1, 0, 0), glm::vec2(0, 0));
		++vert_pos;
		++norm_pos;
	}

	for (size_t i = 0; i < index_count; ++i)
	{
		indices_vec[i] = indices[i];
	}

	return load_model_data(vertices, indices_vec, mat_id);
}

void render_manager::delete_model(const int index)
{
	if (mod_ptr_->find(index) != mod_ptr_->end())
	{
		mod_free_ids_.push(index);

		for (auto && instance : (*ins_ptr_)[index])
		{
			delete_model_instance(instance);
		}

		mod_ptr_->erase(index);
		ins_ptr_->erase(index);
	}
}

void render_manager::register_path(std::string& name, std::string& path_from_root) const
{
	path_manipulator_->register_path(name, path_from_root);
}

void render_manager::register_path(std::string&& name, std::string& path_from_root) const
{
	register_path(name, path_from_root);
}

void render_manager::register_path(std::string& name, std::string&& path_from_root) const
{
	register_path(name, path_from_root);
}

void render_manager::register_path(std::string&& name, std::string&& path_from_root) const
{
	register_path(name, path_from_root);
}

std::string render_manager::get_full_path(const std::string& name) const
{
	return path_manipulator_->get_full_path(name);
}

int render_manager::instance_model(const int index)
{
	if (mod_ptr_ && scn_ptr_ && mod_ptr_->find(index) != mod_ptr_->end())
	{
		const auto instance_id = scn_ptr_->instance_model(&(*mod_ptr_)[index]);
		(*ins_ptr_)[index].push_back(instance_id);

		vertex_count_ += (*mod_ptr_)[index].get_vertex_count();
		poly_count_ += (*mod_ptr_)[index].get_poly_count();

		return instance_id;
	}
	return -1;
}

void render_manager::delete_model_instance(const int index)
{
	auto *m = scn_ptr_->get_instance(index)->m;
	vertex_count_ -= m->get_vertex_count();
	poly_count_ -= m->get_poly_count();
	scn_ptr_->delete_model_instance(index);
}

instance_handle render_manager::get_instance_handle(const int index) const
{
	if (scn_ptr_)
	{
		return instance_handle(scn_ptr_->get_instance(index));
	}
	return instance_handle(nullptr);
}

int render_manager::create_material(const GLuint program, const std::string& filename_texture, const std::string& filename_normals)
{
	//empty_normals
	ASSIGN_FREE_ID(mat_free_ids_, mat_next_free_id_);
	(*mat_ptr_)[id] = material(filename_texture, filename_normals, program);
	
	scn_ptr_->use_material(id);

	return id;
}

int render_manager::create_material(const GLuint program, const glm::vec3& color, const std::string& filename_normals)
{
	ASSIGN_FREE_ID(mat_free_ids_, mat_next_free_id_);
	(*mat_ptr_)[id] = material(color, filename_normals, program);

	scn_ptr_->use_material(id);

	return id;
}

int render_manager::create_material(const GLuint program, const float r, const float g, const float b, const std::string& filename_normals)
{
	return create_material(program, glm::vec3(r, g, b), filename_normals);
}

void render_manager::delete_material(const int index)
{
	if (mat_ptr_->find(index) != mat_ptr_->end())
	{
		mat_free_ids_.push(index);
		mat_ptr_->erase(index);
	}
}

void render_manager::load_environment_cube_map(const std::string& neg_z, const std::string& pos_z,
	const std::string& neg_x, const std::string& pos_x, const std::string& neg_y, const std::string& pos_y) const
{
	// TODO: delete previous environment maps
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

	const float ratio = width / float(height);
	const auto diffuse_id = filter("environment_diffuse_filter.glsl", tex_id, width, height, ratio * 64, 64);

	env_ptr_->set_environment_map(tex_id, diffuse_id);
	rnd_ptr_->enable(vizualization::ENVIRONMENT_MAP);
	//glUseProgram(env_ptr_->get_cube_shader_program());
	//glActiveTexture(GL_TEXTURE31);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

}

void render_manager::load_hdr_environment(const std::string& hdr_img_file, int width, int height, const std::string& ldr_diffuse_file) const
{
	// TODO: delete previous environment maps
	if (!env_ptr_)
		return;

	const std::string hdr_path = path_manipulator_->get_full_path("hdr_textures");
	const std::string ldr_path = path_manipulator_->get_full_path("ldr_textures");

	GLuint tex_id;
	glGenTextures(1, &tex_id);

	glActiveTexture(GL_TEXTURE30);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	
	SOIL_load_OGL_HDR_texture((hdr_path + hdr_img_file).c_str(), SOIL_HDR_RGBdivA, 0, tex_id, 0); // TODO: rescale to max

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint diffuse_id;
	if (ldr_diffuse_file.empty())
	{
		const unsigned int diffuse_size = 64;
		const float ratio = width / float(height);
		const unsigned int diff_width = diffuse_size * ratio;
		diffuse_id = filter("environment_diffuse_filter.glsl", tex_id, width, height, diff_width, diffuse_size);
	}
	else
	{
		unsigned char *image = SOIL_load_image((ldr_path + ldr_diffuse_file).c_str(), &width, &height, nullptr, SOIL_LOAD_RGB);

		glGenTextures(1, &diffuse_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

		SOIL_free_image_data(image);
	}

	env_ptr_->set_environment_map(tex_id, diffuse_id);
	rnd_ptr_->enable(vizualization::ENVIRONMENT_MAP);
}

#define time_now std::chrono::high_resolution_clock::now()

void render_manager::start_framerate()
{
	last_update_ = time_now;
	frame_start_ = time_now;
	frame_count_ = 0;
}

float render_manager::get_fps()
{
	const auto result = frame_count_ / get_seconds(frame_start_);
	frame_start_ = time_now;
	frame_count_ = 0;
	return result;
}

float render_manager::update()
{
	float time_elapsed = 0;

	if (glfwWindowShouldClose(window_) || keys_manager_[GLFW_KEY_ESCAPE])
		should_end_ = true;

	if (valid_ && scn_ptr_ && env_ptr_)
	{
		scn_ptr_->update();

		get_camera().set_aspect(width_ / static_cast<float>(height_));

		time_elapsed = get_seconds(last_update_);
		last_update_ = time_now;
		++frame_count_;
		const auto speed = 5.0f * time_elapsed;

		if (window_visible_)
		{
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
			if (keys_manager_[GLFW_KEY_M] || keys_manager_[GLFW_KEY_DOWN])
				delete render_to_texture(true);
			if (keys_manager_[GLFW_KEY_EQUAL])
				get_camera().set_fov(get_camera().get_fov() + .01);
			if (keys_manager_[GLFW_KEY_MINUS])
				get_camera().set_fov(get_camera().get_fov() - .01);

			if (reset_camera_)
			{
				get_camera().set_transform_forward(glm::vec3(0.0f, 2.0f, 15.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				reset_camera_ = false;
			}
		}

		get_camera().create_matrices();

	}
	if (valid_ && rnd_ptr_ && env_ptr_ && GL_NO_ERROR == glGetError())
	{
		if (window_visible_)
			glfwPollEvents();

		const bool first = env_ptr_->changed();
		env_ptr_->unset_changed();
		for (auto && shd : *shd_ptr_)
			shd.second.update(env_ptr_, first, width_, height_);
	}

	return time_elapsed;
}


void render_manager::render() const
{
	if (valid_ && rnd_ptr_ && scn_ptr_ && env_ptr_ && GL_NO_ERROR == glGetError())
	{
		rnd_ptr_->render(scn_ptr_, mat_ptr_, env_ptr_);
		glfwSwapBuffers(window_);
	}
}

bool render_manager::should_end() const
{
	return should_end_;
}

void render_manager::create_paths(std::string& root_path)
{
	path_manipulator_ = std::make_unique<path_manipulator>(root_path);

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
	if (keys_manager_[GLFW_KEY_P] && action == GLFW_PRESS)
	{
		get_camera().set_perspective(!get_camera().get_perspective());
	}
	if (keys_manager_[GLFW_KEY_I] && action == GLFW_PRESS)
	{
		std::cout << "vertex count: " << vertex_count_ << std::endl;
		std::cout << "poly count: " << poly_count_ << std::endl;
		std::cout << "FPS: " << get_fps() << std::endl;
		std::cout << "fov: " << get_camera().get_fov() << std::endl;
#ifdef MEASURE_SHADER_TIME
		std::cout << "latest render time: " << rnd_ptr_->get_time_elapsed() << " ms" << std::endl;
#endif
	}
}

void render_manager::mouse_move_callback(GLFWwindow* window, const double x_pos, const double y_pos)
{
	const auto resize_const = 500.0f;

	cursor_dx_ = - (x_pos - cursor_x_) / resize_const;
	cursor_dy_ = (y_pos - cursor_y_) / resize_const;

	if (reset_cursor_)
	{
		get_camera().rotate(glm::vec3(0.0f, 1.0f, 0.0f), cursor_dx_);
		get_camera().rotate_local(glm::vec3(1.0f, 0.0f, 0.0f), cursor_dy_);
	}

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

	if (valid_ && rnd_ptr_)
		rnd_ptr_->change_viewport_size(width, height);

	glViewport(0, 0, width, height);
}

float render_manager::get_seconds(const std::chrono::high_resolution_clock::time_point& frame_start) const
{
	const auto frame_end = time_now;
	const auto duration = frame_end - frame_start;
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

	return milliseconds.count() * 0.001f;
}

void render_manager::reorder_pixels(rgba *pixels, const size_t width, const size_t height)
{
	const auto count = width * height;

	const auto temp = new rgba[1];
	for (size_t w = 0; w < width; ++w)
	{
		for (size_t h = 0; h < height / 2; ++h)
		{
			*temp = pixels[h * width + w];
			pixels[h * width + w] = pixels[(height - 1 - h) * width + w];
			pixels[(height - 1 - h) * width + w] = *temp;
		}
	}
	//for (size_t i = 0; i < count / 2; ++i)
	//{
	//	*temp = pixels[i];
	//	pixels[i] = pixels[count - 1 - i];
	//	pixels[count - 1 - i] = *temp;
	//}
	delete[] temp;
}

render_manager::~render_manager()
{
	glfwDestroyWindow(window_);
	if (valid_)
		glfwTerminate();
	//TODO: delete buffer, etc.
}
