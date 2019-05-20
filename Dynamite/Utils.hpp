#pragma once
#include <string>

static inline char *CopyChars(const std::string& str)
{
	// Copy the char data with the terminating null
	auto *ptr = malloc(str.size() + 1);
	memcpy(ptr, str.c_str(), str.size() + 1);
	return (char *)ptr;
}
