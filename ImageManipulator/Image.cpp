#include "Image.hpp"

image& image::operator=(const image& other)
{
	cf_ = other.cf_;
	width_ = other.width_;
	height_ = other.height_;
	bit_depth_ = other.bit_depth_;
	pixels_.resize(other.pixels_.size());
	memcpy(pixels_.data(), other.pixels_.data(), other.pixels_.size());
	return *this;
}

image& image::operator=(image&& other) noexcept
{
	cf_ = other.cf_;
	width_ = other.width_;
	height_ = other.height_;
	bit_depth_ = other.bit_depth_;
	pixels_ = std::move(other.pixels_);
	return *this;
}

image::image(const image& other)
{
	cf_ = other.cf_;
	width_ = other.width_;
	height_ = other.height_;
	bit_depth_ = other.bit_depth_;
	pixels_.resize(other.pixels_.size());
	memcpy(pixels_.data(), other.pixels_.data(), other.pixels_.size());
}

image::image(image&& other) noexcept
{
	cf_ = other.cf_;
	width_ = other.width_;
	height_ = other.height_;
	bit_depth_ = other.bit_depth_;
	pixels_ = std::move(other.pixels_);
}

// TODO: reverse rows if height is positive, negate height if negative (remember to negate height when exporting)
image::image(const ImageFileFormats::color_format& cf, const unsigned int bit_depth, const unsigned int width, const unsigned int height, unsigned char* pixels,
             const unsigned int size): cf_(cf), width_(fabs(width)), height_(fabs(height)), bit_depth_(bit_depth), pixels_(pixels, pixels + size) {}

const std::vector<unsigned char>& image::get_data() const
{
	return pixels_;
}

void image::set_data(unsigned char* pixels, const unsigned int size)
{
	pixels_ = std::vector<unsigned char>(pixels, pixels + size);
}

int image::get_bit_depth() const
{
	return bit_depth_;
}

unsigned image::get_size() const
{
	return pixels_.size();
}

unsigned image::width() const
{
	return width_;
}

unsigned image::height() const
{
	return height_;
}
