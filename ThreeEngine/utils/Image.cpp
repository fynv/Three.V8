#include <cstdlib>
#include <memory.h>
#include "Image.h"
#include "Utils.h"

Image::Image()
{

}

Image::Image(int width, int height)
{	
	m_width = width;
	m_height = height;
	size_t buf_size = (size_t)m_width * (size_t)m_height * 4;
	m_buffer = (uint8_t*)malloc(buf_size);
}

Image::Image(const Image& in)
{	
	m_width = in.m_width;
	m_height = in.m_height;
	size_t buf_size = (size_t)m_width * (size_t)m_height * 4;
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
	size_t buf_size = (size_t)m_width * (size_t)m_height * 4;
	memcpy(m_buffer, in.m_buffer, buf_size);
	return *this;
}
