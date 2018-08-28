#include <fstream>

#include "Scene.hpp"

void split_from(const std::string& str, size_t index, const char c, std::string& before, std::string& after)
{
	while (index < str.size() && str[index] != c)
	{
		before += str[index++];
	}
	if (index + 1 < str.size())
		after = str.substr(index + 1, str.size() - index - 1);
}

scene::scene(const std::string& filename)
{
	std::ifstream ifs(filename);

	std::string line;
	std::string sign;
	while (ifs.good())
	{		
		if (ifs.peek() == 'v')
		{
			ifs >> sign;
			if (sign == "v")
			{
				float x, y, z;
				ifs >> x; ifs >> y; ifs >> z;
				vertices_.emplace_back(x, y, z);
			}
			continue;
		}
		if (ifs.peek() == 'f')
		{
			ifs >> sign;
			int first, second, third;
			ifs >> first; ifs >> second; ifs >> third;
			indices_.push_back(first);
			indices_.push_back(second);
			indices_.push_back(third);
			continue;
		}

		std::getline(ifs, line);
		// TODO: improve
	}

	camera_ = glm::vec3(0, 0, -1);
	
	// =======TESTING=======
	vertices_.resize(3);
	vertices_[0] = glm::vec3(-0.5f, -0.5f, 0.0f);
	vertices_[1] = glm::vec3(0.0f, 0.5f, 0.0f);
	vertices_[2] = glm::vec3(0.5f, -0.5f, 0.0f);
	
	indices_.resize(3);
	indices_[0] = 0;
	indices_[1] = 1;
	indices_[2] = 2;
}

const std::vector<glm::vec3>& scene::get_vertices() const
{
	return vertices_;
}

const std::vector<unsigned short>& scene::get_indices() const
{
	return indices_;
}
