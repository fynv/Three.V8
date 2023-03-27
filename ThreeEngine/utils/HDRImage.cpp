#include <cstdlib>
#include <memory.h>
#include "HDRImage.h"
#include "Utils.h"

HDRImage::HDRImage()
{

}

HDRImage::HDRImage(int width, int height)
{
	m_width = width;
	m_height = height;
	size_t buf_size = (size_t)m_width * (size_t)m_height * 3 * sizeof(float);
	m_buffer = (float*)malloc(buf_size);
}

HDRImage::HDRImage(const HDRImage& in)
{
	m_width = in.m_width;
	m_height = in.m_height;
	size_t buf_size = (size_t)m_width * (size_t)m_height * 3 * sizeof(float);
	m_buffer = (float*)malloc(buf_size);
	memcpy(m_buffer, in.m_buffer, buf_size);
}

HDRImage::~HDRImage()
{
	free(m_buffer);
}

const float* HDRImage::get_data(int& width, int& height) const
{
	width = m_width;
	height = m_height;
	return m_buffer;
}

const HDRImage& HDRImage::operator=(const HDRImage& in)
{
	size_t buf_size = (size_t)m_width * (size_t)m_height * 3 * sizeof(float);
	memcpy(m_buffer, in.m_buffer, buf_size);
	return *this;
}
