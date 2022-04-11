#pragma once

#include <vector>

class Image;
class ImageLoader
{
public:
	static void LoadFile(Image* image, const char* fn, bool keep_alpha = false);
	static void LoadMemory(Image* image, unsigned char* data, size_t size, bool keep_alpha = false);
};

