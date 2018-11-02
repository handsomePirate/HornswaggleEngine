#include <utility>
#include <map>

#include "Renderer.hpp"

renderer::renderer()
{
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glGenBuffers(1, &vio_);

	const auto stride = sizeof(vertex);//sizeof(glm::vec4) + sizeof(glm::vec3) * 3 + sizeof(glm::vec2);
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

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void renderer::render(const std::shared_ptr<scene>& scn_ptr, 
	const std::shared_ptr<std::map<int, material>>& mat_ptr,
	const std::shared_ptr<environment>& env_ptr) const
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio_);

	// Render background environment map
	if (env_ptr->has_env_map())
	{
		glUseProgram(env_ptr->get_shader_program());
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}