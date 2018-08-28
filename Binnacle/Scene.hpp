#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

struct scene
{
	explicit scene(const std::string& filename); // TODO: move scene loading to another project
	scene(const scene& rm) = delete;
	scene& operator=(const scene& rm) = delete;
	scene(scene && rm) = default;
	scene& operator=(scene && rm) = default;
	virtual ~scene() = default;

	const std::vector<glm::vec3>& get_vertices() const;
	const std::vector<unsigned short>& get_indices() const;
private:
	std::vector<glm::vec3> vertices_;
	std::vector<unsigned short> indices_;
	glm::vec3 camera_;
};
