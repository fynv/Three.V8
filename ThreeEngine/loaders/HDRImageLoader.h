#pragma once

#include <cstdint>

class HDRImage;
class HDRCubeImage;
class HDRImageLoader
{
public:
	static void s_flip_x(float* data, int width, int height);

	static void LoadFile(HDRImage* image, const char* fn, bool flip_x = false);
	static void LoadMemory(HDRImage* image, unsigned char* data, size_t size, bool flip_x = false);

	static void LoadCubeFromFile(HDRCubeImage* image, const char* fn_xp, const char* fn_xn,
		const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn, bool flip_x = true);

	static void LoadCubeFromMemory(HDRCubeImage* image,
		unsigned char* data_xp, size_t size_xp, unsigned char* data_xn, size_t size_xn,
		unsigned char* data_yp, size_t size_yp, unsigned char* data_yn, size_t size_yn,
		unsigned char* data_zp, size_t size_zp, unsigned char* data_zn, size_t size_zn, bool flip_x = true);
};

