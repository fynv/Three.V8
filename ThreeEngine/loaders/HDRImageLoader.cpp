#include <glm.hpp>
#include "HDRImageLoader.h"
#include "utils/Image.h"
#include "utils/HDRImage.h"
#include "utils/Utils.h"

#include "stb_image.h"

void HDRImageLoader::s_flip_x(float* data, int width, int height)
{
	glm::vec3* p_data = (glm::vec3*)data;
	glm::vec3 tmp;
	for (int y = 0; y < height; y++, p_data += width)
	{
		for (int x = 0; x < width / 2; x++)
		{
			tmp = p_data[x];
			p_data[x] = p_data[width - 1 - x];
			p_data[width - 1 - x] = tmp;
		}
	}
}


void HDRImageLoader::LoadFile(HDRImage* image, const char* fn, bool flip_x)
{
	if (!exists_test(fn))
	{
		printf("Failed loading %s\n", fn);
		return;
	}

	free(image->m_buffer);

	int chn;
	float* rgb = stbi_loadf(fn, &image->m_width, &image->m_height, &chn, 3);	

	size_t buf_size = (size_t)image->m_width * (size_t)image->m_height * 3 * sizeof(float);
	image->m_buffer = (float*)malloc(buf_size);
	memcpy(image->m_buffer, rgb, buf_size);
	stbi_image_free(rgb);

	if (flip_x)
	{
		s_flip_x(image->m_buffer, image->m_width, image->m_height);
	}
}

void HDRImageLoader::LoadMemory(HDRImage* image, unsigned char* data, size_t size, bool flip_x)
{
	free(image->m_buffer);
	int chn;
	float* rgb = stbi_loadf_from_memory(data, size, &image->m_width, &image->m_height, &chn, 3);
	size_t buf_size = (size_t)image->m_width * (size_t)image->m_height * 3 * sizeof(float);
	image->m_buffer = (float*)malloc(buf_size);
	memcpy(image->m_buffer, rgb, buf_size);
	stbi_image_free(rgb);

	if (flip_x)
	{
		s_flip_x(image->m_buffer, image->m_width, image->m_height);
	}
}

void HDRImageLoader::LoadCubeFromFile(HDRCubeImage* image, const char* fn_xp, const char* fn_xn,
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


void HDRImageLoader::LoadCubeFromMemory(HDRCubeImage* image,
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

inline void add(int count, glm::vec3* hdr, const HDRImageLoader::Range& range, const glm::u8vec4* ldr)
{
	for (int i = 0; i < count; i++)
	{
		glm::vec3 v_hdr = hdr[i];
		glm::u8vec3 s_ldr = ldr[i];
		v_hdr += glm::vec3(s_ldr) / 255.0f * (range.high - range.low) + range.low;
		hdr[i] = v_hdr;
	}
}


void HDRImageLoader::FromImages(HDRImage* image, const Image** ldr, const Range* ranges, int count)
{
	if (count < 1) return;
	free(image->m_buffer);
	image->m_width = ldr[0]->width();
	image->m_height = ldr[0]->height();
	size_t buf_size = (size_t)image->m_width * (size_t)image->m_height * 3 * sizeof(float);
	image->m_buffer = (float*)malloc(buf_size);
	memset(image->m_buffer, 0, buf_size);

	glm::vec3* p_out = (glm::vec3*)image->m_buffer;

	for (int i = 0; i < count; i++)
	{
		const Image* img = ldr[i];
		const Range& range = ranges[i];		
		const glm::u8vec4* p_in = (const glm::u8vec4*)img->data();
		add(image->m_width * image->m_height, p_out, range, p_in);
	}
}