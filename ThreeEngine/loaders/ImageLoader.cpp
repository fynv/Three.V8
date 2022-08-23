#include "ImageLoader.h"
#include "utils/Image.h"
#include "utils/Utils.h"

#include "stb_image.h"

void ImageLoader::LoadFile(Image* image, const char* fn, bool keep_alpha)
{
	if (!exists_test(fn))
	{
		printf("Failed loading %s\n", fn);
		return;
	}

	free(image->m_buffer);

	int chn;
	stbi_uc* rgba = stbi_load(fn, &image->m_width, &image->m_height, &chn, 4);	
	size_t buf_size = (size_t)image->m_width * (size_t)image->m_height * 4;
	image->m_buffer = (uint8_t*)malloc(buf_size);
	memcpy(image->m_buffer, rgba, buf_size);
	stbi_image_free(rgba);

}

void ImageLoader::LoadMemory(Image* image, unsigned char* data, size_t size, bool keep_alpha)
{
	free(image->m_buffer);
	int chn;
	stbi_uc* rgba = stbi_load_from_memory(data, size, &image->m_width, &image->m_height, &chn, 4);
	size_t buf_size = (size_t)image->m_width * (size_t)image->m_height * 4;
	image->m_buffer = (uint8_t*)malloc(buf_size);
	memcpy(image->m_buffer, rgba, buf_size);
	stbi_image_free(rgba);

}


void ImageLoader::LoadCubeFromFile(CubeImage* image, const char* fn_xp, const char* fn_xn, const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn)
{
	LoadFile(&image->images[0], fn_xp);
	LoadFile(&image->images[1], fn_xn);
	LoadFile(&image->images[2], fn_yp);
	LoadFile(&image->images[3], fn_yn);
	LoadFile(&image->images[4], fn_zp);
	LoadFile(&image->images[5], fn_zn);
}

void ImageLoader::LoadCubeFromMemory(CubeImage* image,
	unsigned char* data_xp, size_t size_xp, unsigned char* data_xn, size_t size_xn,
	unsigned char* data_yp, size_t size_yp, unsigned char* data_yn, size_t size_yn,
	unsigned char* data_zp, size_t size_zp, unsigned char* data_zn, size_t size_zn)
{
	LoadMemory(&image->images[0], data_xp, size_xp);
	LoadMemory(&image->images[1], data_xn, size_xn);
	LoadMemory(&image->images[2], data_yp, size_yp);
	LoadMemory(&image->images[3], data_yn, size_yn);
	LoadMemory(&image->images[4], data_zp, size_zp);
	LoadMemory(&image->images[5], data_zn, size_zn);
}
