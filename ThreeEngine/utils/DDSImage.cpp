#include <cstdlib>
#include <memory.h>
#include "DDSImage.h"
#include "Utils.h"

size_t DDSImage::get_size(int width, int height, Format format)
{
	uint32_t bpp_num = 32;
	uint32_t bpp_den = 1;

	if (format == Format::BC1)
	{
		bpp_num = 64;
		bpp_den = 16;
	}
	else if (format == Format::BC2)
	{
		bpp_num = 128;
		bpp_den = 16;
	}
	else if (format == Format::BC3)
	{
		bpp_num = 128;
		bpp_den = 16;
	}
	else if (format == Format::BC4)
	{
		bpp_num = 64;
		bpp_den = 16;
	}
	else if (format == Format::BC5)
	{
		bpp_num = 64;
		bpp_den = 16;
	}
	else if (format == Format::BC6H)
	{
		bpp_num = 128;
		bpp_den = 16;
	}
	else if (format == Format::BC7)
	{
		bpp_num = 128;
		bpp_den = 16;
	}

	if (bpp_den == 1)
	{
		return width * height * 4;
	}
	else
	{
		int w = (width + 3) / 4;
		int h = (height + 3) / 4;
		uint32_t blockSize = bpp_num >> 3;
		return blockSize * w * h;
	}
}

DDSImage::DDSImage()
{

}

DDSImage::DDSImage(int width, int height, Format format)
{
	m_width = width;
	m_height = height;
	m_format = format;
	size_t buf_size = get_size(width, height, format);
	m_buffer = (uint8_t*)malloc(buf_size);
}

DDSImage::DDSImage(const DDSImage& in)
{
	m_width = in.m_width;
	m_height = in.m_height;
	m_format = in.m_format;
	size_t buf_size = get_size(m_width, m_height, m_format);
	m_buffer = (uint8_t*)malloc(buf_size);
	memcpy(m_buffer, in.m_buffer, buf_size);
}

DDSImage::~DDSImage()
{
	free(m_buffer);
}

const DDSImage& DDSImage::operator=(const DDSImage& in)
{
	size_t buf_size = get_size(m_width, m_height, m_format);
	memcpy(m_buffer, in.m_buffer, buf_size);
	return *this;
}
