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

void renderer::render(const std::shared_ptr<scene>& scn_ptr, const std::shared_ptr<std::map<int, material>>& mat_ptr) const
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio_);

	for (auto && mat : *mat_ptr)
	{
		mat.second.update();
		mat.second.set_as_active();
		const auto& vertices = scn_ptr->get_vertices(mat.first);
		if (vertices.empty())
		{
			continue;
		}
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_DYNAMIC_DRAW);

		const auto& indices = scn_ptr->get_indices(mat.first);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_DYNAMIC_DRAW);
		if (!indices.empty())
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr); // keep in mind the unsigned short!!!
																					  //glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}