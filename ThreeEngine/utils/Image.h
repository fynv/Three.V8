#pragma once
#include <cstdint>

class Image
{
public:
	Image();
	Image(int width, int height, bool has_alpha = false);	
	Image(const Image& in);
	~Image();

	bool has_alpha() const { return m_has_alpha; }
	int width() const { return m_width; }
	int height() const { return m_height; }
	const uint8_t* data() const { return m_buffer; }
	uint8_t* data() { return m_buffer; }

	const uint8_t* get_data(int& width, int& height) const;

	const Image& operator=(const Image& in);

	friend class ImageLoader;

private:
	bool m_has_alpha = false;
	int m_width = 0;
	int m_height = 0;
	uint8_t* m_buffer = nullptr;


};
