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
}
