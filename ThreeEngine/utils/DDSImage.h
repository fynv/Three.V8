#pragma once
#include <cstdint>

class DDSImage
{
public:
	enum class Format
	{
		BGRA,
		BC1,
		BC2,
		BC3,
		BC4,
		BC5,
		BC6H,
		BC7,
		FormatCount
	};
	static size_t get_size(int width, int height, Format format);

	DDSImage();
	DDSImage(int width, int height, Format format);
	DDSImage(const DDSImage& in);
	~DDSImage();	

	int width() const { return m_width; }
	int height() const { return m_height; }
	Format format() const { return m_format;  }
	const uint8_t* data() const { return m_buffer; }
	uint8_t* data() { return m_buffer; }

	const DDSImage& operator=(const DDSImage& in);

	friend class DDSImageLoader;

private:
	int m_width = 0;
	int m_height = 0;
	Format m_format = Format::BGRA;
	uint8_t* m_buffer = nullptr;
};

