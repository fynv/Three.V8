#include "ImageLoader.h"
#include "utils/Image.h"
#include "utils/Utils.h"

#include "stb_image.h"

void ImageLoader::s_flip_x(uint8_t* data, int width, int height)
{
	uint32_t* p_data = (uint32_t *)data;
	uint32_t tmp;
	for (int y = 0; y < height; y++, p_data+= width)
	{
		for (int x = 0; x < width / 2; x++)
		{
			tmp = p_data[x];
			p_data[x] = p_data[width - 1 - x];
			p_data[width - 1 - x] = tmp;
		}
	}

}

void ImageLoader::LoadFile(Image* image, const char* fn, bool flip_x)
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

	if (flip_x)
	{
		s_flip_x(image->m_buffer, image->m_width, image->m_height);
	}

}

void ImageLoader::LoadMemory(Image* image, unsigned char* data, size_t size, bool flip_x)
{
	free(image->m_buffer);
	int chn;
	stbi_uc* rgba = stbi_load_from_memory(data, size, &image->m_width, &image->m_height, &chn, 4);
	size_t buf_size = (size_t)image->m_width * (size_t)image->m_height * 4;
	image->m_buffer = (uint8_t*)malloc(buf_size);
	memcpy(image->m_buffer, rgba, buf_size);
	stbi_image_free(rgba);

	if (flip_x)
	{
		s_flip_x(image->m_buffer, image->m_width, image->m_height);
	}

}


void ImageLoader::LoadCubeFromFile(CubeImage* image, const char* fn_xp, const char* fn_xn, 
	const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn, bool flip_x)
{
	if (flip_x)
	{
		LoadFile(&image->images[1], fn_xp, true);
		LoadFile(&image->images[0], fn_xn, true);
		LoadFile(&image->images[2], fn_yp, true);
		LoadFile(&image->images[3], fn_yn, true);
		LoadFile(&image->images[4], fn_zp, true);
		LoadFile(&image->images[5], fn_zn, true);
	}
	else
	{
		LoadFile(&image->images[0], fn_xp, false);
		LoadFile(&image->images[1], fn_xn, false);
		LoadFile(&image->images[2], fn_yp, false);
		LoadFile(&image->images[3], fn_yn, false);
		LoadFile(&image->images[4], fn_zp, false);
		LoadFile(&image->images[5], fn_zn, false);
	}
}

void ImageLoader::LoadCubeFromMemory(CubeImage* image,
	unsigned char* data_xp, size_t size_xp, unsigned char* data_xn, size_t size_xn,
	unsigned char* data_yp, size_t size_yp, unsigned char* data_yn, size_t size_yn,
	unsigned char* data_zp, size_t size_zp, unsigned char* data_zn, size_t size_zn, bool flip_x)
{
	if (flip_x)
	{
		LoadMemory(&image->images[1], data_xp, size_xp, true);
		LoadMemory(&image->images[0], data_xn, size_xn, true);
		LoadMemory(&image->images[2], data_yp, size_yp, true);
		LoadMemory(&image->images[3], data_yn, size_yn, true);
		LoadMemory(&image->images[4], data_zp, size_zp, true);
		LoadMemory(&image->images[5], data_zn, size_zn, true);
	}
	else
	{
		LoadMemory(&image->images[0], data_xp, size_xp, false);
		LoadMemory(&image->images[1], data_xn, size_xn, false);
		LoadMemory(&image->images[2], data_yp, size_yp, false);
		LoadMemory(&image->images[3], data_yn, size_yn, false);
		LoadMemory(&image->images[4], data_zp, size_zp, false);
		LoadMemory(&image->images[5], data_zn, size_zn, false);
	}
}
