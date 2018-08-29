#pragma once
#include <string>

#include <GL/glew.h>

struct texture
{
	std::string name;
	GLuint handle{};

	explicit texture(const std::string& filename);

	texture(const std::string& filename, const std::string& name);

	void bind_to_unit(int unit) const;
};


