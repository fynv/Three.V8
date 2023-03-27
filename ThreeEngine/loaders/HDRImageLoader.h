#pragma once

#include <cstdint>

class HDRImage;
class HDRImageLoader
{
public:
	static void LoadFile(HDRImage* image, const char* fn);
	static void LoadMemory(HDRImage* image, unsigned char* data, size_t size);
};

