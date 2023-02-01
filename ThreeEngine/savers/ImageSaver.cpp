#include <vector>
#include <cstring>
#include "ImageSaver.h"
#include "loaders/ImageLoader.h"
#include "utils/Image.h"

#include "stb_image_write.h"


void ImageSaver::SaveFile(const Image* image, const char* fn, bool flip_x)
{
	const uint8_t* p_data = image->data();
	std::vector<uint8_t> local_data;
	if (flip_x)
	{
		local_data.resize((size_t)image->width() * (size_t)image->height() * 4);
		memcpy(local_data.data(), image->data(), (size_t)image->width() * (size_t)image->height() * 4);
		ImageLoader::s_flip_x(local_data.data(), image->width(), image->height());
		p_data = local_data.data();
	}
	

	if (strlen(fn) > 4 && strcmp(fn + strlen(fn) - 4, ".jpg") == 0)
	{
		stbi_write_jpg(fn, image->width(), image->height(), 4, p_data, 80);
	}
	else
	{
		stbi_write_png(fn, image->width(), image->height(), 4, p_data, image->width() * 4);
	}
}


void ImageSaver::SaveCubeToFile(const CubeImage* image, const char* fn_xp, const char* fn_xn, 
	const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn, bool flip_x)
{
	if (flip_x)
	{
		SaveFile(&image->images[1], fn_xp, true);
		SaveFile(&image->images[0], fn_xn, true);
		SaveFile(&image->images[2], fn_yp, true);
		SaveFile(&image->images[3], fn_yn, true);
		SaveFile(&image->images[4], fn_zp, true);
		SaveFile(&image->images[5], fn_zn, true);
	}
	else
	{
		SaveFile(&image->images[0], fn_xp, false);
		SaveFile(&image->images[1], fn_xn, false);
		SaveFile(&image->images[2], fn_yp, false);
		SaveFile(&image->images[3], fn_yn, false);
		SaveFile(&image->images[4], fn_zp, false);
		SaveFile(&image->images[5], fn_zn, false);
	}
}