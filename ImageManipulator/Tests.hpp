#pragma once
#include "ImageManipulator.hpp"
#include <string>
#include <iostream>

inline void load_test(const std::string& filename)
{
	image img;
	std::string msg;
	if (!image_manipulator::load(filename, img, msg))
	{
		std::cout << msg << std::endl;
	}
	else
	{
		if (!msg.empty())
			std::cout << msg << std::endl;
		std::cout << "Success!" << std::endl;
	}
	image_manipulator::save(img, "copy.bmp", msg);
}

inline void save_test(const std::string& filename, const unsigned int square)
{
	const unsigned int width = square;
	const unsigned int height = square;

	std::vector<unsigned char> pixels(width * height * 3);
	for (unsigned int i = 0; i < height; ++i)
	{
		for (unsigned int j = 0; j < width; ++j)
		{
			const unsigned int pixel_index = i * height + j;
			const unsigned char red_val = j * (255 / float(width));
			const unsigned char green_val = i * (255 / float(height));
			const unsigned char blue_neg_val = red_val > green_val ? red_val : green_val;
			const unsigned char blue_val = 255 - blue_neg_val;
			pixels[pixel_index * 3] = blue_val;
			pixels[pixel_index * 3 + 1] = green_val;
			pixels[pixel_index * 3 + 2] = red_val;
		}
	}
	const image img(ImageFileFormats::BGR, 24, width, height, pixels.data(), pixels.size());
	std::string msg;
	image_manipulator::save(img, filename, msg);
}
