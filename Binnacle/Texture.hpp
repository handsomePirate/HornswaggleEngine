#pragma once
#include <string>

#include <GL/glew.h>

struct texture
{
	std::string name;
	GLuint handle{};

	texture();
	explicit texture(const std::string& filename);
	texture(const std::string& filename, const std::string& name);

	void destroy() const;

	void bind_to_unit(int unit) const;
};


