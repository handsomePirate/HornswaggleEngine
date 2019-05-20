#pragma once
#include "FileFormats.hpp"
#include <vector>

struct pixel_rgba
{
	unsigned int r;
	unsigned int g;
	unsigned int b;
	unsigned int a;
};

struct pixel_rgb
{
	unsigned int r;
	unsigned int g;
	unsigned int b;
};

struct pixel_bw
{
	unsigned int i;
};

struct image
{
	image() = default;
	image& operator=(const image& other);
	image& operator=(image&& other) noexcept;
	image(const image& other);
	image(image&& other) noexcept;
	~image() = default;
	
	image(const ImageFileFormats::color_format& cf, unsigned int bit_depth, unsigned int width, unsigned int height, unsigned char* pixels, unsigned int size);

	const std::vector<unsigned char>& get_data() const;
	void set_data(unsigned char* pixels, unsigned int size);
	int get_bit_depth() const;
	unsigned int get_size() const;
	unsigned int width() const;
	unsigned int height() const;
private:
	ImageFileFormats::color_format cf_{};
	unsigned int width_{};
	unsigned int height_{};
	int bit_depth_{};
	std::vector<unsigned char> pixels_;
};
