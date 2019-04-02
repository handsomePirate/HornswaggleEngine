#include "Renderer.hpp"

renderer::renderer(const GLuint texture_draw_program)
	: texture_draw_program_(texture_draw_program)
{
	glGenBuffers(1, &quad_vbo_);
}

/*
GLuint renderer::filter(const GLuint program, const GLuint tex, const unsigned int in_width, const unsigned int in_height, 
	const unsigned int out_width, const unsigned int out_height) const
{
	vertex vertices[4]
	{
		vertex(glm::vec4(-1, -1, 0, 1), glm::vec3(), glm::vec2(0, 0)),
		vertex(glm::vec4(-1, 1, 0, 1), glm::vec3(), glm::vec2(0, 1)),
		vertex(glm::vec4(1, -1, 0, 1), glm::vec3(), glm::vec2(1, 0)),
		vertex(glm::vec4(1, 1, 0, 1), glm::vec3(), glm::vec2(1, 1))
	};

	unsigned int indices[6]
	{
		0, 2, 1,
		1, 2, 3
	};

	GLuint texture_fbo;
	GLuint rendered_texture;
	GLuint depth_render_buffer;

	glGenFramebuffers(1, &texture_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, texture_fbo);

	// The texture we're going to render to
	glGenTextures(1, &rendered_texture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, rendered_texture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, out_width, out_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// The depth buffer
	glGenRenderbuffers(1, &depth_render_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, out_width, out_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer);

	// Set "rendered_texture_" as our color attachment #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendered_texture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Render to texture error: Framebuffer not correctly set!" << std::endl;

	glViewport(0, 0, out_width, out_height);

	glClearColor(1, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	set_buffer_attrib(buffer_attrib::VERTEX);

	glBindVertexArray(vao_); // TODO: remove - not necessary
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio_);

	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	const auto original_loc = glGetUniformLocation(program, "original");
	glUniform1i(original_loc, 0);

	const auto original_width_loc = glGetUniformLocation(program, "original_width");
	glUniform1i(original_width_loc, in_width);
	const auto original_height_loc = glGetUniformLocation(program, "original_height");
	glUniform1i(original_height_loc, in_height);

	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vertex), vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//======DEBUG SAVE
	//const auto image_data_size = out_height * out_width;
	//const auto image_data = new rgba[image_data_size];
	//glReadPixels(0, 0, out_width, out_height, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<unsigned char *>(&image_data[0]));
	//SOIL_save_image("diffuse.bmp", SOIL_SAVE_TYPE_BMP, out_width, out_height, 4, reinterpret_cast<unsigned char *>(&image_data[0]));
	//delete[] image_data;
	//================

	//glDeleteTextures(1, &rendered_texture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteRenderbuffers(1, &depth_render_buffer);
	glDeleteFramebuffers(1, &texture_fbo);

	return rendered_texture;
}
*/
void renderer::render_texture(const GLuint tex, const int samples) const
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);

	glUseProgram(texture_draw_program_);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	const auto tex_loc = glGetUniformLocation(texture_draw_program_, "tex");
	glUniform1i(tex_loc, 0);

	const auto samples_loc = glGetUniformLocation(texture_draw_program_, "samples");
	glUniform1i(samples_loc, samples);

	glDrawArrays(GL_POINTS, 0, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
