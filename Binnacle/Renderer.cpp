#include <fstream>

#include "Renderer.hpp"
#include <iostream>
#include <utility>

shader::shader()
{
	text_length = 0;
	text = nullptr;
}

shader::shader(GLint text_length, std::string text)
	: text_length(text_length), text(std::move(text))
{
}

renderer::renderer(): program_(0)
{
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glGenBuffers(1, &vio_);

	const auto stride = sizeof(glm::vec4) + sizeof(glm::vec3) * 2 + sizeof(glm::vec2);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(sizeof(glm::vec4)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(sizeof(glm::vec4) + sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(sizeof(glm::vec4) + sizeof(glm::vec3) * 2));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void renderer::update(const std::shared_ptr<environment>& env_ptr_) const
{
	// The camera uniform
	const auto camera_loc = glGetUniformLocation(program_, "camera"); // TODO: move the location retrieval

	glUniform3fv(camera_loc, 1, &env_ptr_->get_camera().get_position()[0]);

	// The transformation matrices uniform
	const auto projection_view_matrix_loc = glGetUniformLocation(program_, "projectionViewMatrix");
	
	const auto view_matrix = env_ptr_->get_camera().get_view_matrix();
	const auto projection_matrix = env_ptr_->get_camera().get_projection_matrix();
	const auto view_projection_matrix = projection_matrix * view_matrix;
	glUniformMatrix4fv(projection_view_matrix_loc, 1, GL_FALSE, &view_projection_matrix[0][0]);

	// The lights uniform
	const auto lights_loc = glGetUniformLocation(program_, "lights");

	const auto lights = env_ptr_->get_lights();
	glUniform3fv(lights_loc, lights.size(), &lights[0][0]);

	// The texture samplers
	const auto tex_sampler_loc = glGetUniformLocation(program_, "texSampler");

	glUniform1i(tex_sampler_loc, 0);
}

void renderer::render(const std::shared_ptr<scene>& scn_ptr) const
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio_);

	const auto& vertices = scn_ptr->get_vertices();
	if (vertices.empty())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		return;
	}
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);

	const auto& indices = scn_ptr->get_indices();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
	if (!indices.empty())
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr); // keep in mind the unsigned short!!!
	//glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

bool renderer::load_shader(shader_type&& type, const std::string& filename)
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

bool renderer::compile_and_link_shaders()
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

	program_ = glCreateProgram();

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

	if (linked && validated)
	{
		glUseProgram(program_);
		return true;
	}

	return false;
}