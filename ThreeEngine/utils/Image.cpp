#include "Image.h"
#include "Utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image(int width, int height, bool has_alpha)
{
	m_has_alpha = has_alpha;
	m_width = width;
	m_height = height;
	size_t buf_size = (size_t)m_width * (size_t)m_height * (m_has_alpha?4:3);
	m_buffer = (uint8_t*)malloc(buf_size);
}


Image::Image(const char* fn, bool keep_alpha)
{
	if (!exists_test(fn))
		printf("Failed loading %s\n", fn);

	int chn;
	stbi_uc* rgba = stbi_load(fn, &m_width, &m_height, &chn, 4);
	m_has_alpha = keep_alpha && chn > 3;
	chn = m_has_alpha ? 4 : 3;
	size_t buf_size = (size_t)m_width * (size_t)m_height * (m_has_alpha ? 4 : 3);
	m_buffer = (uint8_t*)malloc(buf_size);
	for (size_t i = 0; i < (size_t)m_width * (size_t)m_height; i++)
	{
		const uint8_t* p_in = rgba + i * 4;
		uint8_t* p_out = m_buffer + i * chn;
		p_out[0] = p_in[2];
		p_out[1] = p_in[1];
		p_out[2] = p_in[0];
		if (m_has_alpha)
			p_out[3] = p_in[3];
	}
	stbi_image_free(rgba);
}

Image::Image(const Image& in)
{
	m_has_alpha = in.m_has_alpha;
	m_width = in.m_width;
	m_height = in.m_height;
	size_t buf_size = (size_t)m_width * (size_t)m_height * (m_has_alpha ? 4 : 3);
	m_buffer = (uint8_t*)malloc(buf_size);
	memcpy(m_buffer, in.m_buffer, buf_size);
}

Image::~Image()
{
	free(m_buffer);
}

const uint8_t* Image::get_data(int& width, int& height) const
{
	width = m_width;
	height = m_height;
	return m_buffer;
}

const Image& Image::operator=(const Image& in)
{
	size_t buf_size = (size_t)m_width * (size_t)m_height * (m_has_alpha ? 4 : 3);
	memcpy(m_buffer, in.m_buffer, buf_size);
	return *this;
}
