#include <SOIL/SOIL.h>

#include "Texture.hpp"

texture::texture()
{
	handle = -1;
	name = "null";
}

texture::texture(const std::string& filename) : texture(filename, filename) {}

texture::texture(const std::string& filename, const std::string& name)
{
	int width, height;
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, nullptr, SOIL_LOAD_RGB);

	this->name = name;
	glGenTextures(1, &handle);
	bind_to_unit(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	SOIL_free_image_data(image);
}

void texture::bind_to_unit(int unit) const
{
	if (unit > 31 || unit < 0)
		unit = 0;

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, handle);
}
