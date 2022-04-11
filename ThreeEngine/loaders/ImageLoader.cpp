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
	image->m_has_alpha = keep_alpha && chn > 3;
	chn = image->m_has_alpha ? 4 : 3;
	size_t buf_size = (size_t)image->m_width * (size_t)image->m_height * (image->m_has_alpha ? 4 : 3);
	image->m_buffer = (uint8_t*)malloc(buf_size);
	for (size_t i = 0; i < (size_t)image->m_width * (size_t)image->m_height; i++)
	{
		const uint8_t* p_in = rgba + i * 4;
		uint8_t* p_out = image->m_buffer + i * chn;
		p_out[0] = p_in[2];
		p_out[1] = p_in[1];
		p_out[2] = p_in[0];
		if (image->m_has_alpha)
			p_out[3] = p_in[3];
	}
	stbi_image_free(rgba);

}

void ImageLoader::LoadMemory(Image* image, unsigned char* data, size_t size, bool keep_alpha)
{
	free(image->m_buffer);
	int chn;
	stbi_uc* rgba = stbi_load_from_memory(data, size, &image->m_width, &image->m_height, &chn, 4);
	image->m_has_alpha = keep_alpha && chn > 3;
	chn = image->m_has_alpha ? 4 : 3;
	size_t buf_size = (size_t)image->m_width * (size_t)image->m_height * (image->m_has_alpha ? 4 : 3);
	image->m_buffer = (uint8_t*)malloc(buf_size);
	for (size_t i = 0; i < (size_t)image->m_width * (size_t)image->m_height; i++)
	{
		const uint8_t* p_in = rgba + i * 4;
		uint8_t* p_out = image->m_buffer + i * chn;
		p_out[0] = p_in[2];
		p_out[1] = p_in[1];
		p_out[2] = p_in[0];
		if (image->m_has_alpha)
			p_out[3] = p_in[3];
	}
	stbi_image_free(rgba);

}