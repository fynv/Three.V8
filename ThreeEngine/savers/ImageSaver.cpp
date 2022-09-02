#include "ImageSaver.h"
#include "utils/Image.h"

#include "stb_image_write.h"


void ImageSaver::SaveFile(const Image* image, const char* fn)
{
	stbi_write_png(fn, image->width(), image->height(), 4, image->data(), image->width() * 4);
}


void ImageSaver::SaveCubeToFile(const CubeImage* image, const char* fn_xp, const char* fn_xn, const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn)
{
	SaveFile(&image->images[0], fn_xp);
	SaveFile(&image->images[1], fn_xn);
	SaveFile(&image->images[2], fn_yp);
	SaveFile(&image->images[3], fn_yn);
	SaveFile(&image->images[4], fn_zp);
	SaveFile(&image->images[5], fn_zn);
}