#include "ImageManipulator.hpp"
#include <fstream>
#include <cassert>
#include <Windows.h>

bool image_manipulator::load(const std::string& filename, image& img, std::string& msg)
{
	const auto type = recognize_file_type(filename);

	switch (type)
	{
	case ImageFileFormats::BMP:
		if (!load_bmp(filename, img, msg))
			return false;
		if (!check_suffix(filename, ".bmp"))
			msg = "Warning: Incorrect file suffix: BMP!";
		break;

	case ImageFileFormats::PNG:
		if (!load_png(filename, img, msg))
			return false;
		if (!check_suffix(filename, ".png"))
			msg = "Warning: Incorrect file suffix: PNG!";
		break;

	case ImageFileFormats::JPG:
		if (!load_jpg(filename, img, msg))
			return false;
		if (!check_suffix(filename, ".jpg"))
			msg = "Warning: Incorrect file suffix: JPG!";
		break;

	case ImageFileFormats::HDR:
		if (!load_hdr(filename, img, msg))
			return false;
		if (!check_suffix(filename, ".hdr"))
			msg = "Warning: Incorrect file suffix: hdr!";
		break;

	case ImageFileFormats::Not_Present:
		msg = "Error: There is no such file!";
		return false;

	default:
		msg = "Error: Unknown file format!";
		return false;
	}
	return true;
}

bool image_manipulator::save(const image& img, const std::string& filename, std::string& msg)
{
	const auto type = get_file_type(filename);

	switch (type)
	{
	case ImageFileFormats::BMP:
		if (!save_bmp(filename, img, msg))
			return false;
		break;

	case ImageFileFormats::PNG:
		if (!save_png(filename, img, msg))
			return false;
		break;

	case ImageFileFormats::JPG:
		if (!save_jpg(filename, img, msg))
			return false;
		break;

	case ImageFileFormats::HDR:
		if (!save_hdr(filename, img, msg))
			return false;
		break;

	default:
		msg = "Error: Unknown file format!";
		return false;
	}
	return true;
}

#define CHECK_IFSTREAM(ifs, ret) if ((ifs).bad() || (ifs).eof()) return ret
#define IFSTREAM_RETURN(type) ifs.close(); return (type)
ImageFileFormats::file_type image_manipulator::recognize_file_type(const std::string& filename)
{
	std::ifstream ifs(filename);
	if (!ifs.is_open())
		return ImageFileFormats::Not_Present;
	char c;
	ifs.get(c);
	CHECK_IFSTREAM(ifs, ImageFileFormats::Unknown);
	if (c == 'B')
	{
		//! We are matching the BMP file
		ifs.get(c);
		CHECK_IFSTREAM(ifs, ImageFileFormats::Unknown);
		if (c == 'M')
		{
			IFSTREAM_RETURN(ImageFileFormats::BMP);
		}
		IFSTREAM_RETURN(ImageFileFormats::Unknown);
	}
	if (c == 89)
	{
		//! We are matching the PNG file
		ifs.get(c);
		CHECK_IFSTREAM(ifs, ImageFileFormats::Unknown);
		if (c == 'P')
		{
			ifs.get(c);
			CHECK_IFSTREAM(ifs, ImageFileFormats::Unknown);
			if (c == 'N')
			{
				ifs.get(c);
				CHECK_IFSTREAM(ifs, ImageFileFormats::Unknown);
				if (c == 'G')
				{
					IFSTREAM_RETURN(ImageFileFormats::PNG);
				}
			}
		}
		IFSTREAM_RETURN(ImageFileFormats::Unknown);
	}

	IFSTREAM_RETURN(ImageFileFormats::Unknown);
}

ImageFileFormats::file_type image_manipulator::get_file_type(const std::string& filename)
{
	const unsigned int max = filename.size() - 1;
	unsigned int i = max;
	for (; i > 0 && filename[i] != '.'; --i) {}
	if (filename[i] != '.')
		return ImageFileFormats::Unknown;
	if (i + 3 == max && filename[i + 1] == 'b' && filename[i + 2] == 'm' && filename[i + 3] == 'p')
		return ImageFileFormats::BMP;
	if (i + 3 == max && filename[i + 1] == 'p' && filename[i + 2] == 'n' && filename[i + 3] == 'g')
		return ImageFileFormats::PNG;
	if (i + 3 == max && filename[i + 1] == 'j' && filename[i + 2] == 'p' && filename[i + 3] == 'g')
		return ImageFileFormats::JPG;
	if (i + 3 == max && filename[i + 1] == 'h' && filename[i + 2] == 'd' && filename[i + 3] == 'r')
		return ImageFileFormats::HDR;
	return ImageFileFormats::Unknown;
}

#define FREAD_SHORT(s, f) fread_s(&s, sizeof(s), sizeof(s), 1, f)
#define FILE_RETURN(f, type) fclose(f); return (type)

bool image_manipulator::load_bmp(const std::string& filename, image& img, std::string& msg)
{
	//std::basic_ifstream<uint8_t> ifs(filename, std::ios_base::binary | std::ios_base::in);
	FILE *f;
	fopen_s(&f, filename.c_str(), "rb");

	ImageFileFormats::bmp_header_structure h{};
	fread_s(&h, sizeof(ImageFileFormats::bmp_header_structure), sizeof(ImageFileFormats::bmp_header_structure), 1, f);
	const uint32_t dib_size = fgetc(f);
	fseek(f, 14, 0);
	assert(h.type == 0x4d42);

	// TODO: padding
	if (dib_size == 12)
	{
		ImageFileFormats::dib_header_12 dh{};
		FREAD_SHORT(dh, f);
		const unsigned int size = h.size - h.offset;
		unsigned int padding;
		unsigned char *pixels = read_pixels(f, h.offset, size, dh.width_px * dh.bits_per_pixel / 8, dh.height_px, padding);
		img = image(ImageFileFormats::BGR, dh.bits_per_pixel, dh.width_px, dh.height_px, pixels, size - padding);
		delete[] pixels;
		FILE_RETURN(f, true);
	}
	if (dib_size == 16)
	{
		ImageFileFormats::dib_header_16 dh{};
		FREAD_SHORT(dh, f);
		const unsigned int size = h.size - h.offset;
		unsigned int padding;
		unsigned char *pixels = read_pixels(f, h.offset, size, dh.width_px * dh.bits_per_pixel / 8, dh.height_px, padding);
		img = image(ImageFileFormats::BGR, dh.bits_per_pixel, dh.width_px, dh.height_px, pixels, size - padding);
		delete[] pixels;
		FILE_RETURN(f, true);
		// TODO: compression
	}
	if (dib_size == 40)
	{
		ImageFileFormats::dib_header_40 dh{};
		FREAD_SHORT(dh, f);
		unsigned int padding;
		unsigned char *pixels = read_pixels(f, h.offset, dh.image_size_bytes, dh.width_px * dh.bits_per_pixel / 8, dh.height_px, padding);
		img = image(ImageFileFormats::BGR, dh.bits_per_pixel, dh.width_px, dh.height_px, pixels, dh.image_size_bytes - padding);
		delete[] pixels;
		FILE_RETURN(f, true);
		// TODO: compression
	}
	if (dib_size == 52 || dib_size == 56 || dib_size == 64)
	{
		ImageFileFormats::dib_header_52 dh{};
		FREAD_SHORT(dh, f);
		unsigned int padding;
		unsigned char *pixels = read_pixels(f, h.offset, dh.image_size_bytes, dh.width_px * dh.bits_per_pixel / 8, dh.height_px, padding);
		img = image(ImageFileFormats::BGR, dh.bits_per_pixel, dh.width_px, dh.height_px, pixels, dh.image_size_bytes - padding);
		delete[] pixels;
		FILE_RETURN(f, true);
		// TODO: compression
	}
	if (dib_size == 108)
	{
		ImageFileFormats::dib_header_108 dh{};
		FREAD_SHORT(dh, f);
		unsigned int padding;
		unsigned char *pixels = read_pixels(f, h.offset, dh.image_size_bytes, dh.width_px * dh.bits_per_pixel / 8, dh.height_px, padding);
		img = image(ImageFileFormats::BGR, dh.bits_per_pixel, dh.width_px, dh.height_px, pixels, dh.image_size_bytes - padding);
		delete[] pixels;
		FILE_RETURN(f, true);
		// TODO: compression
	}
	if (dib_size == 124)
	{
		ImageFileFormats::dib_header_124 dh{};
		FREAD_SHORT(dh, f);
		unsigned int padding;
		unsigned char *pixels = read_pixels(f, h.offset, dh.image_size_bytes, dh.width_px * dh.bits_per_pixel / 8, dh.height_px, padding);
		img = image(ImageFileFormats::BGR, dh.bits_per_pixel, dh.width_px, dh.height_px, pixels, dh.image_size_bytes - padding);
		delete[] pixels;
		FILE_RETURN(f, true);
		// TODO: compression
	}

	FILE_RETURN(f, false);
}

bool image_manipulator::load_png(const std::string& filename, image& img, std::string& msg)
{
	return false;
}

bool image_manipulator::load_jpg(const std::string& filename, image& img, std::string& msg)
{
	return false;
}

bool image_manipulator::load_hdr(const std::string& filename, image& img, std::string& msg)
{
	return false;
}

bool image_manipulator::save_bmp(const std::string& filename, const image& img, std::string& msg)
{
	ImageFileFormats::bmp_header_structure h{};
	const unsigned int img_byte_size = img.get_size();
	unsigned int padding = 0;
	const unsigned int byte_row_width = img.width() * img.get_bit_depth() / 8;
	while ((byte_row_width + padding) % 4 != 0)
		++padding;

	h.type = 0x4d42;
	h.size = 54 + img_byte_size + padding * img.height();
	h.offset = 54;
	ImageFileFormats::dib_header_40 dh{};
	dh.dib_header_size = 40;
	dh.bits_per_pixel = img.get_bit_depth();
	dh.width_px = img.width();
	dh.height_px = img.height();
	dh.plane_count = 1;
	dh.image_size_bytes = img_byte_size + padding * img.height();
	auto& pixels = img.get_data();

	FILE *f;
	// "wb" means binary, otherwise, the output prefixes every 0x0a value with 0x0d, i.e. disallows writing newline without carriage return
	fopen_s(&f, filename.c_str(), "wb");
	fwrite(&h, sizeof(ImageFileFormats::bmp_header_structure), 1, f);
	fwrite(&dh, sizeof(ImageFileFormats::dib_header_40), 1, f);

	// TODO: will not work for negative height -- change
	for (unsigned int i = 0; i < img.height(); ++i)
	{
		fwrite((unsigned char *)&pixels[i * byte_row_width], 1, byte_row_width, f);
		for (unsigned int k = 0; k < padding; ++k)
			fputc(unsigned char(0), f);
	}

	//ofs.close();
	fclose(f);
	return true;
}

bool image_manipulator::save_png(const std::string& filename, const image& img, std::string& msg)
{
	return false;
}

bool image_manipulator::save_jpg(const std::string& filename, const image& img, std::string& msg)
{
	return false;
}

bool image_manipulator::save_hdr(const std::string& filename, const image& img, std::string& msg)
{
	return false;
}

bool image_manipulator::check_suffix(const std::string& filename, const std::string& expected)
{
	if (expected.size() > filename.size())
		return false;

	for (unsigned int i = 1; i <= expected.size(); ++i)
		if (tolower(expected[expected.size() - i]) != tolower(filename[filename.size() - i]))
			return false;

	return true;
}

unsigned char* image_manipulator::read_pixels(FILE *f, const unsigned int start, const unsigned int count, const unsigned int width, const unsigned int height, unsigned int& padding)
{
	padding = count - width * height;
	const unsigned int actual_count = count - padding;
	auto *result = new unsigned char[actual_count];

	fseek(f, start, 0);
	//ifs.seekg(start);
	const unsigned int row_padding = padding / height;
	for (unsigned int i = 0; i < actual_count; ++i)
	{
		if (i > 0 && i % width == 0)
		{
			for (unsigned int j = 0; j < row_padding; ++j)
			{
				fgetc(f);
			}
		}
		result[i] = fgetc(f);
	}

	return result;
}
