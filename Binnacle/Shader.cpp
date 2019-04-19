#include <fstream>
#include <iostream>
#include <utility>

#include "Shader.hpp"

shader::shader()
	: text_length(0), text(nullptr) {}

shader::shader(const GLint text_length, std::string text)
	: text_length(text_length), text(std::move(text)) {}

shader_program::shader_program()
{
	program_ = glCreateProgram();
}

shader_program::shader_program(bool)
{ }

bool shader_program::load_shader(shader_type&& type, const std::string& filename)
{
	std::string shader_text;

	std::ifstream ifs(filename);
	std::string line;

	if (!ifs.good() || !ifs.is_open())
		return false;

	while (std::getline(ifs, line).good())
	{
		shader_text += line + '\n';
	}
	shader_text += line + '\n';

	shaders_.insert_or_assign(type, shader_text);

	return true;
}

#define CHECK_SHADER_LOG(sh, text)\
{GLint maxLength = 0; \
glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &maxLength); \
std::vector<GLchar> infoLog(maxLength); \
glGetInfoLogARB(sh, maxLength, &maxLength, &infoLog[0]); \
std::cout << (text) << std::endl; \
for (auto && ch : infoLog)\
std::cout << static_cast<char>(ch); \
std::cout << std::endl; }


#define CHECK_PROGRAM_LOG(program, text)\
{GLint maxLength = 0;\
glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);\
std::vector<GLchar> infoLog(maxLength);\
glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);\
std::cout << (text) << std::endl;\
for (auto && ch : infoLog)\
std::cout << static_cast<char>(ch);\
std::cout << std::endl;}

bool shader_program::compile_and_link_shaders(const bool pipeline)
{
	if (!pipeline)
	{
		const GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);

		const GLchar *compute_text = shaders_[COMPUTE].c_str();
		const GLint compute_length = shaders_[COMPUTE].length();
		glShaderSource(compute_shader, 1, &compute_text, &compute_length);
		glCompileShader(compute_shader);

		GLint compiled;

		glGetObjectParameterivARB(compute_shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			CHECK_SHADER_LOG(compute_shader, "compute shader:");
			return false;
		}
		std::cout << "Compute shader loaded." << std::endl;

		glAttachShader(program_, compute_shader);
		glLinkProgram(program_);

		GLint linked;
		glGetProgramiv(program_, GL_LINK_STATUS, &linked);

		if (!linked)
		{
			CHECK_PROGRAM_LOG(program_, "program linking:");
			return false;
		}

		glDeleteShader(compute_shader);

		return true;
	}

	if (shaders_.find(VERTEX) == shaders_.end() || shaders_.find(FRAGMENT) == shaders_.end())
	{
		// TODO: log error
		return false;
	}

	const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar *vertex_text = shaders_[VERTEX].c_str();
	const GLint vertex_length = shaders_[VERTEX].length();
	glShaderSourceARB(vertex_shader, 1, &vertex_text, &vertex_length);
	const GLchar *fragment_text = shaders_[FRAGMENT].c_str();
	const GLint fragment_length = shaders_[FRAGMENT].length();
	glShaderSourceARB(fragment_shader, 1, &fragment_text, &fragment_length);

	glCompileShaderARB(vertex_shader);
	glCompileShaderARB(fragment_shader);
	//TODO: add geometry shader

	GLint compiled;

	glGetObjectParameterivARB(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		CHECK_SHADER_LOG(vertex_shader, "vertex shader:");
		return false;
	}
	std::cout << "Vertex shader loaded." << std::endl;

	glGetObjectParameterivARB(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		CHECK_SHADER_LOG(fragment_shader, "fragment shader:");
		return false;
	}
	std::cout << "Fragment shader loaded." << std::endl;

	glAttachShader(program_, vertex_shader);
	glAttachShader(program_, fragment_shader);

	GLuint geometry_shader = 0;
	bool has_geometry_shader = false;
	if (shaders_.find(GEOMETRY) != shaders_.end())
	{
		has_geometry_shader = true;
		geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);

		const GLchar *geometry_text = shaders_[GEOMETRY].c_str();
		const GLint geomtery_length = shaders_[GEOMETRY].length();
		glShaderSourceARB(geometry_shader, 1, &geometry_text, &geomtery_length);

		glCompileShaderARB(geometry_shader);

		glGetObjectParameterivARB(geometry_shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			CHECK_SHADER_LOG(geometry_shader, "geometry shader:");
			return false;
		}
		std::cout << "Geometry shader loaded." << std::endl;

		glAttachShader(program_, geometry_shader);
	}

	glLinkProgram(program_);

	GLint linked;
	glGetProgramiv(program_, GL_LINK_STATUS, &linked);
	
	if (!linked)
	{
		CHECK_PROGRAM_LOG(program_, "program linking:");
		return false;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	if (has_geometry_shader)
		glDeleteShader(geometry_shader);
	// TODO: unload shaders

	return true;
}

bool shader_program::validate() const
{
	glValidateProgram(program_);

	GLint validated;
	glGetProgramiv(program_, GL_VALIDATE_STATUS, &validated);

	if (!validated)
	{
		CHECK_PROGRAM_LOG(program_, "program validating:");
		return false;
	}
	return true;
}

GLuint shader_program::get_id() const
{
	return program_;
}

void shader_program::update(const std::shared_ptr<environment>& env_ptr, const bool first, const int width, const int height) const
{
	glUseProgram(program_);

	// The camera uniform
	const auto camera_loc = glGetUniformLocation(program_, "camera"); // TODO: move the location retrieval
	glUniform3fv(camera_loc, 1, &env_ptr->get_camera().get_position()[0]);

	// The transformation matrices uniform
	const auto projection_view_matrix_loc = glGetUniformLocation(program_, "projectionViewMatrix");
	const auto view_matrix = env_ptr->get_camera().get_view_matrix();
	const auto projection_matrix = env_ptr->get_camera().get_projection_matrix();
	const auto view_projection_matrix = projection_matrix * view_matrix;
	glUniformMatrix4fv(projection_view_matrix_loc, 1, GL_FALSE, &view_projection_matrix[0][0]);

	const auto window_width_loc = glGetUniformLocation(program_, "windowWidth");
	glUniform1i(window_width_loc, width);

	const auto window_height_loc = glGetUniformLocation(program_, "windowHeight");
	glUniform1i(window_height_loc, height);

	if (first)
	{
		env_ptr->shader_load_env_map(program_);

		//TODO: exchange for a buffer
		// The lights uniform
		const auto light_pos_loc = glGetUniformLocation(program_, "lightPositions");
		const auto light_pos = env_ptr->get_light_positions();
		if (!light_pos.empty())
			glUniform3fv(light_pos_loc, light_pos.size(), &light_pos[0][0]);

		const int light_count_limit = 20;
		const auto lights_count_loc = glGetUniformLocation(program_, "lightsCount");
		if (!light_pos.empty())
			glUniform1i(lights_count_loc, light_pos.size());

		const auto light_col_loc = glGetUniformLocation(program_, "lightColors");
		const auto light_col = env_ptr->get_light_colors();
		if (!light_col.empty())
			glUniform3fv(light_col_loc, light_col.size(), &light_col[0][0]);

		const auto lights_diff_loc = glGetUniformLocation(program_, "lightIntensities");
		const auto lights_diff = env_ptr->get_light_intensities();
		if (!lights_diff.empty())
			glUniform1fv(lights_diff_loc, lights_diff.size(), &lights_diff[0]);
	}
}

material::material()
	: use_texture_(false), color_(0), specular_color_(1), program_(0), initialized_(false) {}

material::material(const GLuint program)
	: use_texture_(true), color_(0), specular_color_(1), program_(program), initialized_(true) {}

material::material(const std::string& filename_texture, const GLuint program)
	: texture_(filename_texture), use_texture_(true), color_(0), specular_color_(1), program_(program), initialized_(true) {}

material::material(const std::string& filename_texture, const std::string& filename_normals, const GLuint program)
	: texture_(filename_texture), normal_map_(filename_normals), use_texture_(true), color_(1), specular_color_(1), program_(program), initialized_(true) {}

material::material(const glm::vec3& color, const GLuint program)
	: use_texture_(false), color_(color), specular_color_(1), program_(program), initialized_(true) {}

material::material(const glm::vec3& color, const std::string& filename_normals, const GLuint program)
	: normal_map_(filename_normals), use_texture_(false), color_(color), specular_color_(1), program_(program), initialized_(true) {}

material::material(const glm::vec3& specular_color, const glm::vec3& color, GLuint program)
	: use_texture_(false), color_(color), specular_color_(specular_color), program_(program), initialized_(true) {}

material::material(const glm::vec3& specular_color, const glm::vec3& color, const std::string& filename_normals, GLuint program)
	: normal_map_(filename_normals), use_texture_(false), color_(color), specular_color_(specular_color), program_(program), initialized_(true) {}

void material::set_as_active() const
{
	if (initialized_)
	{
		glUseProgram(program_);

		texture_.bind_to_unit(0);
		normal_map_.bind_to_unit(2);
	}
}

void material::use_program(const GLuint program)
{
	program_ = program;
	initialized_ = true;
}

void material::update()
{
	// TODO: create the material polymorphism to be able to use both classic Phong and PBR
	const auto use_texture_loc = glGetUniformLocation(program_, "useTexture");
	glUniform1i(use_texture_loc, use_texture_);
	
	const auto mat_color_loc = glGetUniformLocation(program_, "material.color");
	glUniform3fv(mat_color_loc, 1, &color_[0]);

	const float roughness = 0.01f;
	const float metalness = 0;
	const float index_of_refraction = 1;

	const auto roughness_loc = glGetUniformLocation(program_, "material.roughness");
	glUniform1f(roughness_loc, roughness);

	const auto metalness_loc = glGetUniformLocation(program_, "material.metalness");
	glUniform1f(metalness_loc, metalness);

	const auto index_of_refraction_loc = glGetUniformLocation(program_, "material.n");
	glUniform1f(index_of_refraction_loc, index_of_refraction);

	// The texture samplers
	const auto diffuse_map_loc = glGetUniformLocation(program_, "diffuseMap");
	glUniform1i(diffuse_map_loc, 0);

	const auto normal_map_loc = glGetUniformLocation(program_, "normalMap");
	glUniform1i(normal_map_loc, 2);
}
