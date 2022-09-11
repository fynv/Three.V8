#pragma once
#include <cstdint>

class Image
{
public:
	Image();
	Image(int width, int height);	
	Image(const Image& in);
	~Image();

	int width() const { return m_width; }
	int height() const { return m_height; }
	const uint8_t* data() const { return m_buffer; }
	uint8_t* data() { return m_buffer; }

	const uint8_t* get_data(int& width, int& height) const;

	const Image& operator=(const Image& in);

	friend class ImageLoader;
	friend class GLRenderTarget;
	friend class CubeRenderTarget;

private:
	int m_width = 0;
	int m_height = 0;
	uint8_t* m_buffer = nullptr;

};

class CubeImage
{
public:
	Image images[6];
};


