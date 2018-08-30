#pragma once
#include <iostream>

enum bit_mask {
	b1 = static_cast<unsigned int>(1 << 0),
	b2 = static_cast<unsigned int>(1 << 1),
	b3 = static_cast<unsigned int>(1 << 2),
	b4 = static_cast<unsigned int>(1 << 3),
	b5 = static_cast<unsigned int>(1 << 4),
	b6 = static_cast<unsigned int>(1 << 5),
	b7 = static_cast<unsigned int>(1 << 6),
	b8 = static_cast<unsigned int>(1 << 7),
	b9 = static_cast<unsigned int>(1 << 8),
	b10 = static_cast<unsigned int>(1 << 9),
	b11 = static_cast<unsigned int>(1 << 10),
	b12 = static_cast<unsigned int>(1 << 11),
	b13 = static_cast<unsigned int>(1 << 12),
	b14 = static_cast<unsigned int>(1 << 13),
	b15 = static_cast<unsigned int>(1 << 14),
	b16 = static_cast<unsigned int>(1 << 15),
	b17 = static_cast<unsigned int>(1 << 16),
	b18 = static_cast<unsigned int>(1 << 17),
	b19 = static_cast<unsigned int>(1 << 18),
	b20 = static_cast<unsigned int>(1 << 19),
	b21 = static_cast<unsigned int>(1 << 20),
	b22 = static_cast<unsigned int>(1 << 21),
	b23 = static_cast<unsigned int>(1 << 22),
	b24 = static_cast<unsigned int>(1 << 23),
	b25 = static_cast<unsigned int>(1 << 24),
	b26 = static_cast<unsigned int>(1 << 25),
	b27 = static_cast<unsigned int>(1 << 26),
	b28 = static_cast<unsigned int>(1 << 27),
	b29 = static_cast<unsigned int>(1 << 28),
	b30 = static_cast<unsigned int>(1 << 29),
	b31 = static_cast<unsigned int>(1 << 30),
	b32 = static_cast<unsigned int>(1 << 31),
	all = static_cast<unsigned int>(0xFFFFFFFF)
};

static unsigned int get_bit(const bit_mask bit, const unsigned int *p)
{
	return bit & *p ? 1 : 0;
}

static void write_all_bits(const unsigned int *p)
{
	std::cout
		<< get_bit(b1, p) << get_bit(b2, p) << get_bit(b3, p) << get_bit(b4, p)
		<< get_bit(b5, p) << get_bit(b6, p) << get_bit(b7, p) << get_bit(b8, p)
		<< get_bit(b9, p) << get_bit(b10, p) << get_bit(b11, p) << get_bit(b12, p)
		<< get_bit(b13, p) << get_bit(b14, p) << get_bit(b15, p) << get_bit(b16, p)
		<< get_bit(b17, p) << get_bit(b18, p) << get_bit(b19, p) << get_bit(b20, p)
		<< get_bit(b21, p) << get_bit(b22, p) << get_bit(b23, p) << get_bit(b24, p)
		<< get_bit(b25, p) << get_bit(b26, p) << get_bit(b27, p) << get_bit(b28, p)
		<< get_bit(b29, p) << get_bit(b30, p) << get_bit(b31, p) << get_bit(b32, p)
	<< std::endl;
}

static void set_bit(const unsigned int bit, unsigned int *p, const bool set)
{
	if (set)
		*p = (bit & all) | *p;
	else
		*p = ~bit & *p;
}