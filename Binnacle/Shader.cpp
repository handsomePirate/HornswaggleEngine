#include <fstream>
#include <iostream>

#include "Shader.hpp"

shader::shader()
	: text_length(0), text(nullptr) {}

shader::shader(const GLint text_length, const std::string&  text)
	: text_length(text_length), text(std::move(text)) {}

shader_program::shader_program()
{
	program_ = glCreateProgram();
}

bool shader_program::load_shader(shader_type&& type, const std::string& filename)
{
	std::string shader_text;

	std::ifstream ifs(filename);
	std::string line;

	if (!ifs.good())
		return false;

	while (std::getline(ifs, line).good())
	{
		shader_text += line + '\n';
	}
	shader_text += line + '\n';

	shaders_.insert_or_assign(type, shader_text); // shader_text.length() -

	return true;
}


bool shader_program::compile_and_link_shaders()
{
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


	GLint compiled;

	glGetObjectParameterivARB(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		GLint blen = 0;
		GLsizei slen = 0;

		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &blen);
		if (blen > 1)
		{
			auto *compiler_log = static_cast<GLchar *>(malloc(blen));
			glGetInfoLogARB(vertex_shader, blen, &slen, compiler_log);
			std::cout << "vertex shader:" << std::endl << compiler_log << std::endl;
			free(compiler_log);
		}
		return false;
	}
	std::cout << "Vertex shader loaded." << std::endl;

	glGetObjectParameterivARB(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		GLint blen = 0;
		GLsizei slen = 0;

		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &blen);
		if (blen > 1)
		{
			auto *compiler_log = static_cast<GLchar *>(malloc(blen));
			glGetInfoLogARB(fragment_shader, blen, &slen, compiler_log);
			std::cout << "fragment shader:" << std::endl << compiler_log << std::endl;
			free(compiler_log);
		}
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
			GLint blen = 0;
			GLsizei slen = 0;

			glGetShaderiv(geometry_shader, GL_INFO_LOG_LENGTH, &blen);
			if (blen > 1)
			{
				auto* compiler_log = static_cast<GLchar *>(malloc(blen));
				glGetInfoLogARB(geometry_shader, blen, &slen, compiler_log);
				std::cout << "geometry shader:" << std::endl << compiler_log << std::endl;
				free(compiler_log);
			}
			return false;
		}
		std::cout << "Geometry shader loaded." << std::endl;

		glAttachShader(program_, geometry_shader);
	}

	glLinkProgram(program_);

	GLint linked;
	glGetProgramiv(program_, GL_LINK_STATUS, &linked);

	glValidateProgram(program_);

	GLint validated;
	glGetProgramiv(program_, GL_VALIDATE_STATUS, &validated);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	if (has_geometry_shader)
		glDeleteShader(geometry_shader);

	return linked && validated;
}

GLuint shader_program::get_id() const
{
	return program_;
}

void shader_program::update(const std::shared_ptr<environment>& env_ptr) const
{
	// The camera uniform
	const auto camera_loc = glGetUniformLocation(program_, "camera"); // TODO: move the location retrieval
	glUniform3fv(camera_loc, 1, &env_ptr->get_camera().get_position()[0]);

	// The transformation matrices uniform
	const auto projection_view_matrix_loc = glGetUniformLocation(program_, "projectionViewMatrix");
	const auto view_matrix = env_ptr->get_camera().get_view_matrix();
	const auto projection_matrix = env_ptr->get_camera().get_projection_matrix();
	const auto view_projection_matrix = projection_matrix * view_matrix;
	glUniformMatrix4fv(projection_view_matrix_loc, 1, GL_FALSE, &view_projection_matrix[0][0]);

	// The lights uniform
	const auto lights_loc = glGetUniformLocation(program_, "lights");
	const auto lights = env_ptr->get_lights();
	glUniform3fv(lights_loc, lights.size(), &lights[0][0]);

	const auto lights_count_loc = glGetUniformLocation(program_, "lightsCount");
	glUniform1i(lights_count_loc, lights.size());
}

material::material()
	: use_texture_(false), color_(0.0f, 0.0f, 0.0f), program_(0), initialized_(false) {}

material::material(const GLuint program)
	: use_texture_(true), color_(0.0f, 0.0f, 0.0f), program_(program), initialized_(true) {}

material::material(const std::string& filename_texture, const GLuint program)
	: texture_(filename_texture), use_texture_(true), color_(0.0f, 0.0f, 0.0f), program_(program), initialized_(true) {}

material::material(const glm::vec3& color, const GLuint program)
	: use_texture_(false), color_(color), program_(program), initialized_(true) {}

void material::set_as_active() const
{
	if (initialized_)
	{
		glUseProgram(program_);

		if (use_texture_)
			texture_.bind_to_unit(0);
	}
}

void material::use_program(const GLuint program)
{
	program_ = program;
	initialized_ = true;
}

void material::update()
{
	const auto use_texture_loc = glGetUniformLocation(program_, "useTexture");
	glUniform1i(use_texture_loc, use_texture_);

	const auto mat_color_loc = glGetUniformLocation(program_, "matColor");
	glUniform3fv(mat_color_loc, 1, &color_[0]);

	// The texture samplers
	const auto tex_sampler_loc = glGetUniformLocation(program_, "texSampler");
	glUniform1i(tex_sampler_loc, 0);
}
