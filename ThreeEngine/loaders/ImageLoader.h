#pragma once

#include <cstdint>

class Image;
class CubeImage;
class ImageLoader
{
public:
	static void s_flip_x(uint8_t* data, int width, int height);

	static void LoadFile(Image* image, const char* fn, bool flip_x = false);
	static void LoadMemory(Image* image, unsigned char* data, size_t size, bool flip_x = false);

	static void LoadCubeFromFile(CubeImage* image, const char* fn_xp, const char* fn_xn, 
		const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn, bool flip_x = true);

	static void LoadCubeFromMemory(CubeImage* image,
		unsigned char* data_xp, size_t size_xp, unsigned char* data_xn, size_t size_xn,
		unsigned char* data_yp, size_t size_yp, unsigned char* data_yn, size_t size_yn,
		unsigned char* data_zp, size_t size_zp, unsigned char* data_zn, size_t size_zn, bool flip_x = true);
};

