#include "HDRImageLoader.h"
#include "utils/HDRImage.h"
#include "utils/Utils.h"

#include "stb_image.h"


void HDRImageLoader::LoadFile(HDRImage* image, const char* fn)
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
}

void HDRImageLoader::LoadMemory(HDRImage* image, unsigned char* data, size_t size)
{
	free(image->m_buffer);
	int chn;
	float* rgb = stbi_loadf_from_memory(data, size, &image->m_width, &image->m_height, &chn, 3);
	size_t buf_size = (size_t)image->m_width * (size_t)image->m_height * 3 * sizeof(float);
	image->m_buffer = (float*)malloc(buf_size);
	memcpy(image->m_buffer, rgb, buf_size);
	stbi_image_free(rgb);
}