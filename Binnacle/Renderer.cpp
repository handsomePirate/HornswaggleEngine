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

renderer::renderer()
{
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glGenBuffers(1, &vio_);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void renderer::render(const std::shared_ptr<scene>& scn_ptr) const
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio_);

	const auto& vertices = scn_ptr->get_vertices();
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	const auto& indices = scn_ptr->get_indices();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
	if (!indices.empty())
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr);
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

	const auto program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

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

		glAttachShader(program, geometry_shader);
	}

	glLinkProgram(program);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	glValidateProgram(program);

	GLint validated;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &validated);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	if (has_geometry_shader)
		glDeleteShader(geometry_shader);

	if (linked && validated)
	{
		glUseProgram(program);
		return true;
	}

	return false;
}