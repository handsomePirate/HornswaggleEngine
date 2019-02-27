#pragma once
#include <string>

#include <GL/glew.h>

// The pixel form structure
struct rgba
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

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


