#include <utility>
#include <map>

#include "OpenGLRenderer.hpp"
#include <iostream>

opengl_renderer::opengl_renderer(const GLuint texture_draw_program)
	: renderer(texture_draw_program)
{
	vizualization_options_ = new bool[2]
	{
		/*LIGHTS = */false,
		/*ENVIRONMENT_MAP = */false
	};

	glGenBuffers(1, &vbo_);
	glGenBuffers(1, &vio_);

	opengl_renderer::set_buffer_attrib(buffer_attrib::VERTEX);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	background_color_ = glm::vec3(0.1f, 0.1f, 0.1f);
}

opengl_renderer::~opengl_renderer()
{
	delete[] vizualization_options_;
}

void opengl_renderer::render(const std::shared_ptr<scene>& scn_ptr,
	const std::shared_ptr<std::map<int, material>>& mat_ptr,
	const std::shared_ptr<environment>& env_ptr) const
{
	glClearColor(background_color_.r, background_color_.g, background_color_.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	set_buffer_attrib(buffer_attrib::VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio_);

	// Render background environment map
	if (env_ptr && is_enabled(vizualization::ENVIRONMENT_MAP))
	{
		glUseProgram(env_ptr->get_cube_shader_program());
		auto& cube_model = env_ptr->get_environment_cube();
		vertex vertices[8];
		const auto vertex_data = cube_model.get_vertex_data();

		const auto& cam_pos = env_ptr->get_camera().get_position();
		for (size_t i = 0; i < 8; i++)
		{
			vertices[i] = vertex_data[i];
			vertices[i].translate(cam_pos.x, cam_pos.y, cam_pos.z);
		}
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(vertex), vertices, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), cube_model.get_index_data(), GL_DYNAMIC_DRAW);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

		glClear(GL_DEPTH_BUFFER_BIT);
	}
	for (auto && mat : *mat_ptr)
	{
		mat.second.set_as_active();
		mat.second.update();

		const auto& vertices = scn_ptr->get_vertices(mat.first);
		if (vertices.empty())
			continue;

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_DYNAMIC_DRAW);

		const auto& indices = scn_ptr->get_indices(mat.first);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);
		if (!indices.empty())
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr); // keep in mind the unsigned int!!!
	}
	if (env_ptr && is_enabled(vizualization::LIGHTS))
	{
		glUseProgram(env_ptr->get_lights_shader_program());
		auto& light_visuals = env_ptr->get_light_visuals();

		if (!light_visuals.empty())
		{
			glBufferData(GL_ARRAY_BUFFER, light_visuals.size() * sizeof(vertex), &light_visuals[0], GL_DYNAMIC_DRAW);
			glDrawArrays(GL_POINTS, 0, light_visuals.size());
		}
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void opengl_renderer::enable(const vizualization& option) const
{
	vizualization_options_[static_cast<int>(option)] = true;
}

void opengl_renderer::disable(const vizualization& option) const
{
	vizualization_options_[static_cast<int>(option)] = false;
}

bool opengl_renderer::is_enabled(const vizualization& option) const
{
	return vizualization_options_[static_cast<int>(option)];
}

void opengl_renderer::set_buffer_attrib(const buffer_attrib& option) const
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	unsigned stride;

	switch (option)
	{
	default:
		/*WARNING: this branch falls through to the next one to make the vertex attribution default*/
	case buffer_attrib::VERTEX:
		stride = sizeof(vertex);//sizeof(glm::vec4) + sizeof(glm::vec3) * 3 + sizeof(glm::vec2);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(sizeof(glm::vec4)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(sizeof(glm::vec4) + sizeof(glm::vec3)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(sizeof(glm::vec4) + sizeof(glm::vec3) * 2));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(sizeof(glm::vec4) + sizeof(glm::vec3) * 2 + sizeof(glm::vec2)));
		glEnableVertexAttribArray(4);
		break;
	case buffer_attrib::POINT_LIGHT:
		// TODO
		// A point light needs:
		//	position
		//	intensity - proportional to displayed size
		//	color
		break;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void opengl_renderer::set_background_color(const glm::vec3& color)
{
	background_color_ = color;
}

void opengl_renderer::change_viewport_size(unsigned width, unsigned height)
{ }
