#pragma once
#include "Image.hpp"
#include "FileFormats.hpp"
#include <string>

struct image_manipulator
{
	static bool load(const std::string& filename, image& img, std::string& msg);
	static bool save(const image& img, const std::string& filename, std::string& msg);
private:
	static ImageFileFormats::file_type recognize_file_type(const std::string& filename);
	static ImageFileFormats::file_type get_file_type(const std::string& filename);

	static bool load_bmp(const std::string& filename, image& img, std::string& msg);
	static bool load_png(const std::string& filename, image& img, std::string& msg);
	static bool load_jpg(const std::string& filename, image& img, std::string& msg);
	static bool load_hdr(const std::string& filename, image& img, std::string& msg);

	static bool save_bmp(const std::string& filename, const image& img, std::string& msg);
	static bool save_png(const std::string& filename, const image& img, std::string& msg);
	static bool save_jpg(const std::string& filename, const image& img, std::string& msg);
	static bool save_hdr(const std::string& filename, const image& img, std::string& msg);

	static bool check_suffix(const std::string& filename, const std::string& expected);
	static unsigned char *read_pixels(FILE *f, unsigned int start, unsigned int count, unsigned int width, unsigned int height, unsigned int& padding);
	template<typename T>
	static void write_attribute(FILE* f, const T& attribute);
};

template <typename T>
void image_manipulator::write_attribute(FILE* f, const T& attribute)
{
	const unsigned int byte_size = sizeof(T);

	auto *byte_ptr = (unsigned char *)&attribute;

	for (unsigned int i = 0; i < byte_size; ++i)
	{
		fputc(*byte_ptr, f);
		++byte_ptr;
	}
}
