#pragma once
#include <cstdint>

class HDRImage
{
public:
	HDRImage();
	HDRImage(int width, int height);
	HDRImage(const HDRImage& in);
	~HDRImage();

	int width() const { return m_width; }
	int height() const { return m_height; }
	const float* data() const { return m_buffer; }
	float* data() { return m_buffer; }

	const float* get_data(int& width, int& height) const;

	const HDRImage& operator=(const HDRImage& in);

	friend class Lightmap;
	friend class HDRImageLoader;
	friend class CubeRenderTarget;
	
private:
	int m_width = 0;
	int m_height = 0;
	float* m_buffer = nullptr;

};

class HDRCubeImage
{
public:
	HDRImage images[6];
};


