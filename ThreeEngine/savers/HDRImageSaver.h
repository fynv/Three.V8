#pragma once

class HDRImage;
class HDRCubeImage;
class HDRImageSaver
{
public:
	static void SaveFile(const HDRImage* image, const char* fn, bool flip_x = false);
	static void SaveCubeToFile(const HDRCubeImage* image, const char* fn_xp, const char* fn_xn,
		const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn, bool flip_x = true);
};

