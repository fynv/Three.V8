#include "DDSImageLoader.h"
#include "utils/DDSImage.h"
#include "utils/Utils.h"

#if !defined(MAKEFOURCC)
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
    (unsigned(ch0) | (unsigned(ch1) << 8) | \
    (unsigned(ch2) << 16) | (unsigned(ch3) << 24 ))
#endif

enum DXGI_FORMAT
{
	DXGI_FORMAT_UNKNOWN = 0,

	DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
	DXGI_FORMAT_R32G32B32A32_UINT = 3,
	DXGI_FORMAT_R32G32B32A32_SINT = 4,

	DXGI_FORMAT_R32G32B32_TYPELESS = 5,
	DXGI_FORMAT_R32G32B32_FLOAT = 6,
	DXGI_FORMAT_R32G32B32_UINT = 7,
	DXGI_FORMAT_R32G32B32_SINT = 8,

	DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM = 11,
	DXGI_FORMAT_R16G16B16A16_UINT = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM = 13,
	DXGI_FORMAT_R16G16B16A16_SINT = 14,

	DXGI_FORMAT_R32G32_TYPELESS = 15,
	DXGI_FORMAT_R32G32_FLOAT = 16,
	DXGI_FORMAT_R32G32_UINT = 17,
	DXGI_FORMAT_R32G32_SINT = 18,

	DXGI_FORMAT_R32G8X24_TYPELESS = 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,

	DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
	DXGI_FORMAT_R10G10B10A2_UNORM = 24,
	DXGI_FORMAT_R10G10B10A2_UINT = 25,

	DXGI_FORMAT_R11G11B10_FLOAT = 26,

	DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
	DXGI_FORMAT_R8G8B8A8_UNORM = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	DXGI_FORMAT_R8G8B8A8_UINT = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM = 31,
	DXGI_FORMAT_R8G8B8A8_SINT = 32,

	DXGI_FORMAT_R16G16_TYPELESS = 33,
	DXGI_FORMAT_R16G16_FLOAT = 34,
	DXGI_FORMAT_R16G16_UNORM = 35,
	DXGI_FORMAT_R16G16_UINT = 36,
	DXGI_FORMAT_R16G16_SNORM = 37,
	DXGI_FORMAT_R16G16_SINT = 38,

	DXGI_FORMAT_R32_TYPELESS = 39,
	DXGI_FORMAT_D32_FLOAT = 40,
	DXGI_FORMAT_R32_FLOAT = 41,
	DXGI_FORMAT_R32_UINT = 42,
	DXGI_FORMAT_R32_SINT = 43,

	DXGI_FORMAT_R24G8_TYPELESS = 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,

	DXGI_FORMAT_R8G8_TYPELESS = 48,
	DXGI_FORMAT_R8G8_UNORM = 49,
	DXGI_FORMAT_R8G8_UINT = 50,
	DXGI_FORMAT_R8G8_SNORM = 51,
	DXGI_FORMAT_R8G8_SINT = 52,

	DXGI_FORMAT_R16_TYPELESS = 53,
	DXGI_FORMAT_R16_FLOAT = 54,
	DXGI_FORMAT_D16_UNORM = 55,
	DXGI_FORMAT_R16_UNORM = 56,
	DXGI_FORMAT_R16_UINT = 57,
	DXGI_FORMAT_R16_SNORM = 58,
	DXGI_FORMAT_R16_SINT = 59,

	DXGI_FORMAT_R8_TYPELESS = 60,
	DXGI_FORMAT_R8_UNORM = 61,
	DXGI_FORMAT_R8_UINT = 62,
	DXGI_FORMAT_R8_SNORM = 63,
	DXGI_FORMAT_R8_SINT = 64,
	DXGI_FORMAT_A8_UNORM = 65,

	DXGI_FORMAT_R1_UNORM = 66,

	DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,

	DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM = 69,

	DXGI_FORMAT_BC1_TYPELESS = 70,
	DXGI_FORMAT_BC1_UNORM = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB = 72,

	DXGI_FORMAT_BC2_TYPELESS = 73,
	DXGI_FORMAT_BC2_UNORM = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB = 75,

	DXGI_FORMAT_BC3_TYPELESS = 76,
	DXGI_FORMAT_BC3_UNORM = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB = 78,

	DXGI_FORMAT_BC4_TYPELESS = 79,
	DXGI_FORMAT_BC4_UNORM = 80,
	DXGI_FORMAT_BC4_SNORM = 81,

	DXGI_FORMAT_BC5_TYPELESS = 82,
	DXGI_FORMAT_BC5_UNORM = 83,
	DXGI_FORMAT_BC5_SNORM = 84,

	DXGI_FORMAT_B5G6R5_UNORM = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM = 88,

	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,

	DXGI_FORMAT_BC6H_TYPELESS = 94,
	DXGI_FORMAT_BC6H_UF16 = 95,
	DXGI_FORMAT_BC6H_SF16 = 96,

	DXGI_FORMAT_BC7_TYPELESS = 97,
	DXGI_FORMAT_BC7_UNORM = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB = 99,

	// adding ASTC formats, once appeared in MS document, then disappeared - Fei Yang
	DXGI_FORMAT_ASTC_4X4_UNORM = 134,
	DXGI_FORMAT_ASTC_4X4_UNORM_SRGB = 135,
	DXGI_FORMAT_ASTC_5X4_TYPELESS = 137,
	DXGI_FORMAT_ASTC_5X4_UNORM = 138,
	DXGI_FORMAT_ASTC_5X4_UNORM_SRGB = 139,
	DXGI_FORMAT_ASTC_5X5_TYPELESS = 141,
	DXGI_FORMAT_ASTC_5X5_UNORM = 142,
	DXGI_FORMAT_ASTC_5X5_UNORM_SRGB = 143,
	DXGI_FORMAT_ASTC_6X5_TYPELESS = 145,
	DXGI_FORMAT_ASTC_6X5_UNORM = 146,
	DXGI_FORMAT_ASTC_6X5_UNORM_SRGB = 147,
	DXGI_FORMAT_ASTC_6X6_TYPELESS = 149,
	DXGI_FORMAT_ASTC_6X6_UNORM = 150,
	DXGI_FORMAT_ASTC_6X6_UNORM_SRGB = 151,
	DXGI_FORMAT_ASTC_8X5_TYPELESS = 153,
	DXGI_FORMAT_ASTC_8X5_UNORM = 154,
	DXGI_FORMAT_ASTC_8X5_UNORM_SRGB = 155,
	DXGI_FORMAT_ASTC_8X6_TYPELESS = 157,
	DXGI_FORMAT_ASTC_8X6_UNORM = 158,
	DXGI_FORMAT_ASTC_8X6_UNORM_SRGB = 159,
	DXGI_FORMAT_ASTC_8X8_TYPELESS = 161,
	DXGI_FORMAT_ASTC_8X8_UNORM = 162,
	DXGI_FORMAT_ASTC_8X8_UNORM_SRGB = 163,
	DXGI_FORMAT_ASTC_10X5_TYPELESS = 165,
	DXGI_FORMAT_ASTC_10X5_UNORM = 166,
	DXGI_FORMAT_ASTC_10X5_UNORM_SRGB = 167,
	DXGI_FORMAT_ASTC_10X6_TYPELESS = 169,
	DXGI_FORMAT_ASTC_10X6_UNORM = 170,
	DXGI_FORMAT_ASTC_10X6_UNORM_SRGB = 171,
	DXGI_FORMAT_ASTC_10X8_TYPELESS = 173,
	DXGI_FORMAT_ASTC_10X8_UNORM = 174,
	DXGI_FORMAT_ASTC_10X8_UNORM_SRGB = 175,
	DXGI_FORMAT_ASTC_10X10_TYPELESS = 177,
	DXGI_FORMAT_ASTC_10X10_UNORM = 178,
	DXGI_FORMAT_ASTC_10X10_UNORM_SRGB = 179,
	DXGI_FORMAT_ASTC_12X10_TYPELESS = 181,
	DXGI_FORMAT_ASTC_12X10_UNORM = 182,
	DXGI_FORMAT_ASTC_12X10_UNORM_SRGB = 183,
	DXGI_FORMAT_ASTC_12X12_TYPELESS = 185,
	DXGI_FORMAT_ASTC_12X12_UNORM = 186,
	DXGI_FORMAT_ASTC_12X12_UNORM_SRGB = 187,
};

struct DDSPixelFormat
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourcc;
	uint32_t bitcount;
	uint32_t rmask;
	uint32_t gmask;
	uint32_t bmask;
	uint32_t amask;
};

struct DDSHeader
{
	uint32_t fourcc;
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitch;
	uint32_t depth;
	uint32_t mipmapcount;
	uint32_t reserved[11];
	DDSPixelFormat pf;
	uint32_t caps1;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
	uint32_t notused;
};

struct DDSHeader10
{
	uint32_t dxgiFormat;
	uint32_t resourceDimension;
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t miscFlags2;
};

enum DDPF
{
	DDPF_ALPHAPIXELS = 0x00000001U,
	DDPF_ALPHA = 0x00000002U,
	DDPF_FOURCC = 0x00000004U,
	DDPF_RGB = 0x00000040U,
	DDPF_PALETTEINDEXED1 = 0x00000800U,
	DDPF_PALETTEINDEXED2 = 0x00001000U,
	DDPF_PALETTEINDEXED4 = 0x00000008U,
	DDPF_PALETTEINDEXED8 = 0x00000020U,
	DDPF_LUMINANCE = 0x00020000U,
	DDPF_ALPHAPREMULT = 0x00008000U,

	DDPF_NORMAL = 0x80000000U, /// Custom NVTT flags.
	DDPF_SRGB = 0x40000000U, /// Custom NVTT flags.
};


enum D3DFORMAT
{
	/// 32 bit RGB formats.
	D3DFMT_R8G8B8 = 20,
	/// 32 bit RGB formats.
	D3DFMT_A8R8G8B8 = 21,
	/// 32 bit RGB formats.
	D3DFMT_X8R8G8B8 = 22,
	/// 32 bit RGB formats.
	D3DFMT_R5G6B5 = 23,
	/// 32 bit RGB formats.
	D3DFMT_X1R5G5B5 = 24,
	/// 32 bit RGB formats.
	D3DFMT_A1R5G5B5 = 25,
	/// 32 bit RGB formats.
	D3DFMT_A4R4G4B4 = 26,
	/// 32 bit RGB formats.
	D3DFMT_R3G3B2 = 27,
	/// 32 bit RGB formats.
	D3DFMT_A8 = 28,
	/// 32 bit RGB formats.
	D3DFMT_A8R3G3B2 = 29,
	/// 32 bit RGB formats.
	D3DFMT_X4R4G4B4 = 30,
	/// 32 bit RGB formats.
	D3DFMT_A2B10G10R10 = 31,
	/// 32 bit RGB formats.
	D3DFMT_A8B8G8R8 = 32,
	/// 32 bit RGB formats.
	D3DFMT_X8B8G8R8 = 33,
	/// 32 bit RGB formats.
	D3DFMT_G16R16 = 34,
	/// 32 bit RGB formats.
	D3DFMT_A2R10G10B10 = 35,

	D3DFMT_A16B16G16R16 = 36,

	/// Palette formats.
	D3DFMT_A8P8 = 40,
	/// Palette formats.
	D3DFMT_P8 = 41,

	/// Luminance formats.
	D3DFMT_L8 = 50,
	/// Luminance formats.
	D3DFMT_A8L8 = 51,
	/// Luminance formats.
	D3DFMT_A4L4 = 52,
	/// Luminance formats.
	D3DFMT_L16 = 81,

	/// Floating point formats
	D3DFMT_R16F = 111,
	/// Floating point formats
	D3DFMT_G16R16F = 112,
	/// Floating point formats
	D3DFMT_A16B16G16R16F = 113,
	/// Floating point formats
	D3DFMT_R32F = 114,
	/// Floating point formats
	D3DFMT_G32R32F = 115,
	/// Floating point formats
	D3DFMT_A32B32G32R32F = 116,
};

enum FOURCC
{
	FOURCC_NVTT = MAKEFOURCC('N', 'V', 'T', 'T'),
	FOURCC_DDS = MAKEFOURCC('D', 'D', 'S', ' '),
	FOURCC_DXT1 = MAKEFOURCC('D', 'X', 'T', '1'),
	FOURCC_DXT2 = MAKEFOURCC('D', 'X', 'T', '2'),
	FOURCC_DXT3 = MAKEFOURCC('D', 'X', 'T', '3'),
	FOURCC_DXT4 = MAKEFOURCC('D', 'X', 'T', '4'),
	FOURCC_DXT5 = MAKEFOURCC('D', 'X', 'T', '5'),
	FOURCC_RXGB = MAKEFOURCC('R', 'X', 'G', 'B'),
	FOURCC_ATI1 = MAKEFOURCC('A', 'T', 'I', '1'),
	FOURCC_BC4U = MAKEFOURCC('B', 'C', '4', 'U'),
	FOURCC_BC4S = MAKEFOURCC('B', 'C', '4', 'S'),
	FOURCC_ATI2 = MAKEFOURCC('A', 'T', 'I', '2'),
	FOURCC_BC5U = MAKEFOURCC('B', 'C', '5', 'U'),
	FOURCC_BC5S = MAKEFOURCC('B', 'C', '5', 'S'),
	FOURCC_A2XY = MAKEFOURCC('A', '2', 'X', 'Y'),
	FOURCC_DX10 = MAKEFOURCC('D', 'X', '1', '0'),
	FOURCC_UVER = MAKEFOURCC('U', 'V', 'E', 'R'),
};

void DDSImageLoader::_read_bgra(DDSImage* image, FILE* fp, DDSHeader& header, DDSHeader10& header10)
{
	bool has_header10 = header.pf.fourcc == FOURCC_DX10;
	unsigned has_alpha = false;
	unsigned swizzle = 0; // 0: BRGA 1:RGBA 2:ABGR 3: ARGB

	if (has_header10)
	{
		switch (header10.dxgiFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
			has_alpha = true;
			swizzle = 1;
			break;
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			has_alpha = true;
			swizzle = 0;
			break;
		}
	}
	else if ((header.pf.flags & DDPF_FOURCC) != 0)
	{
		switch (header.pf.fourcc)
		{
		case D3DFMT_R8G8B8:
			has_alpha = false;
			swizzle = 0;
			break;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			has_alpha = true;
			swizzle = 0;
			break;
		case D3DFMT_A8B8G8R8:
		case D3DFMT_X8B8G8R8:
			has_alpha = true;
			swizzle = 1;
			break;
		}
	}
	else
	{
		has_alpha = ((header.pf.flags & (DDPF_ALPHAPIXELS | DDPF_ALPHA)) != 0);
		if (has_alpha)
		{
			if (header.pf.amask == 0xff)
			{
				if (header.pf.rmask == 0xff00)
				{
					swizzle = 3;
				}
				else
				{
					swizzle = 2;
				}
			}
			else
			{
				if (header.pf.rmask == 0xff)
				{
					swizzle = 1;
				}
				else
				{
					swizzle = 0;
				}
			}
		}
		else
		{
			if (header.pf.rmask == 0xff)
			{
				swizzle = 1;
			}
			else
			{
				swizzle = 0;
			}
		}
	}

	size_t size_out = DDSImage::get_size(image->m_width, image->m_height, image->m_format);
	image->m_buffer = (uint8_t*)malloc(size_out);
	size_t num_pixels = size_out / 4;
	size_t size_pixel_in = has_alpha ? 4 : 3;
	uint8_t* p_out = image->m_buffer;
	for (size_t i = 0; i < num_pixels; i++, p_out += 4)
	{
		uint8_t pix_in[4];
		fread(pix_in, 1, size_pixel_in, fp);
		if (swizzle == 0)
		{
			p_out[0] = pix_in[0];
			p_out[1] = pix_in[1];
			p_out[2] = pix_in[2];
			p_out[3] = has_alpha ? pix_in[3] : 255;
		}
		else if (swizzle == 1)
		{
			p_out[0] = pix_in[2];
			p_out[1] = pix_in[1];
			p_out[2] = pix_in[0];
			p_out[3] = has_alpha ? pix_in[3] : 255;
		}
		else if (swizzle == 2)
		{
			p_out[0] = pix_in[1];
			p_out[1] = pix_in[2];
			p_out[2] = pix_in[3];
			p_out[3] = pix_in[0];
		}
		else if (swizzle == 3)
		{
			p_out[0] = pix_in[3];
			p_out[1] = pix_in[2];
			p_out[2] = pix_in[1];
			p_out[3] = pix_in[0];
		}
	}

}

void DDSImageLoader::_read_blocks(DDSImage* image, FILE* fp, DDSHeader& header, DDSHeader10& header10)
{
	size_t size = DDSImage::get_size(image->m_width, image->m_height, image->m_format);
	image->m_buffer = (uint8_t*)malloc(size);
	fread(image->m_buffer, 1, size, fp);
}

void DDSImageLoader::LoadFile(DDSImage* image, const char* fn)
{
	if (!exists_test(fn))
	{
		printf("Failed loading %s\n", fn);
		return;
	}
	free(image->m_buffer);

	FILE* fp = fopen(fn, "rb");
	DDSHeader header;
	fread(&header, sizeof(DDSHeader), 1, fp);

	bool has_header10 = header.pf.fourcc == FOURCC_DX10;
	DDSHeader10 header10;
	if (has_header10)
	{
		fread(&header10, sizeof(DDSHeader10), 1, fp);
	}

	image->m_format = DDSImage::Format::BGRA;
	switch (header.pf.fourcc)
	{
	case FOURCC_DXT1:
		image->m_format = DDSImage::Format::BC1;
		break;
	case FOURCC_DXT3:
		image->m_format = DDSImage::Format::BC2;
		break;
	case FOURCC_DXT5:
		image->m_format = DDSImage::Format::BC3;
		break;
	case FOURCC_BC4U:
		image->m_format = DDSImage::Format::BC4;
		break;
	case FOURCC_BC5U:
		image->m_format = DDSImage::Format::BC5;
		break;
	}

	if (has_header10)
	{		
		switch (header10.dxgiFormat)
		{
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			image->m_format = DDSImage::Format::BC1;
			break;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			image->m_format = DDSImage::Format::BC2;
			break;
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			image->m_format = DDSImage::Format::BC3;
			break;
		case DXGI_FORMAT_BC4_UNORM:
			image->m_format = DDSImage::Format::BC4;
			break;
		case DXGI_FORMAT_BC5_UNORM:
			image->m_format = DDSImage::Format::BC5;
			break;
		case DXGI_FORMAT_BC6H_UF16:
			image->m_format = DDSImage::Format::BC6H;
			break;
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			image->m_format = DDSImage::Format::BC7;
			break;
		}
	}
	image->m_width = header.width;
	image->m_height = header.height;

	if (image->m_format == DDSImage::Format::BGRA)
	{
		_read_bgra(image, fp, header, header10);
	}
	else
	{
		_read_blocks(image, fp, header, header10);
	}


	fclose(fp);
}

void DDSImageLoader::_read_bgra(DDSImage* image, unsigned char* &ptr, DDSHeader& header, DDSHeader10& header10)
{
	bool has_header10 = header.pf.fourcc == FOURCC_DX10;
	unsigned has_alpha = false;
	unsigned swizzle = 0; // 0: BRGA 1:RGBA 2:ABGR 3: ARGB

	if (has_header10)
	{
		switch (header10.dxgiFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
			has_alpha = true;
			swizzle = 1;
			break;
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			has_alpha = true;
			swizzle = 0;
			break;
		}
	}
	else if ((header.pf.flags & DDPF_FOURCC) != 0)
	{
		switch (header.pf.fourcc)
		{
		case D3DFMT_R8G8B8:
			has_alpha = false;
			swizzle = 0;
			break;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			has_alpha = true;
			swizzle = 0;
			break;
		case D3DFMT_A8B8G8R8:
		case D3DFMT_X8B8G8R8:
			has_alpha = true;
			swizzle = 1;
			break;
		}
	}
	else
	{
		has_alpha = ((header.pf.flags & (DDPF_ALPHAPIXELS | DDPF_ALPHA)) != 0);
		if (has_alpha)
		{
			if (header.pf.amask == 0xff)
			{
				if (header.pf.rmask == 0xff00)
				{
					swizzle = 3;
				}
				else
				{
					swizzle = 2;
				}
			}
			else
			{
				if (header.pf.rmask == 0xff)
				{
					swizzle = 1;
				}
				else
				{
					swizzle = 0;
				}
			}
		}
		else
		{
			if (header.pf.rmask == 0xff)
			{
				swizzle = 1;
			}
			else
			{
				swizzle = 0;
			}
		}
	}

	size_t size_out = DDSImage::get_size(image->m_width, image->m_height, image->m_format);
	image->m_buffer = (uint8_t*)malloc(size_out);

	size_t num_pixels = size_out / 4;
	size_t size_pixel_in = has_alpha ? 4 : 3;
	uint8_t* p_out = image->m_buffer;
	for (size_t i = 0; i < num_pixels; i++, p_out += 4)
	{
		uint8_t pix_in[4];
		memcpy(pix_in, ptr, size_pixel_in); ptr += size_pixel_in;
		if (swizzle == 0)
		{
			p_out[0] = pix_in[0];
			p_out[1] = pix_in[1];
			p_out[2] = pix_in[2];
			p_out[3] = has_alpha ? pix_in[3] : 255;
		}
		else if (swizzle == 1)
		{
			p_out[0] = pix_in[2];
			p_out[1] = pix_in[1];
			p_out[2] = pix_in[0];
			p_out[3] = has_alpha ? pix_in[3] : 255;
		}
		else if (swizzle == 2)
		{
			p_out[0] = pix_in[1];
			p_out[1] = pix_in[2];
			p_out[2] = pix_in[3];
			p_out[3] = pix_in[0];
		}
		else if (swizzle == 3)
		{
			p_out[0] = pix_in[3];
			p_out[1] = pix_in[2];
			p_out[2] = pix_in[1];
			p_out[3] = pix_in[0];
		}
	}
}

void DDSImageLoader::_read_blocks(DDSImage* image, unsigned char* &ptr, DDSHeader& header, DDSHeader10& header10)
{
	size_t size = DDSImage::get_size(image->m_width, image->m_height, image->m_format);
	image->m_buffer = (uint8_t*)malloc(size);
	memcpy(image->m_buffer, ptr, size); ptr += size;
}

void DDSImageLoader::LoadMemory(DDSImage* image, unsigned char* data, size_t size)
{
	free(image->m_buffer);
	unsigned char* ptr = data;
	DDSHeader header;
	memcpy(&header, ptr, sizeof(DDSHeader)); ptr += sizeof(DDSHeader);

	bool has_header10 = header.pf.fourcc == FOURCC_DX10;
	DDSHeader10 header10;
	if (has_header10)
	{
		memcpy(&header10, ptr, sizeof(DDSHeader10)); ptr += sizeof(DDSHeader10);
	}

	image->m_format = DDSImage::Format::BGRA;
	switch (header.pf.fourcc)
	{
	case FOURCC_DXT1:
		image->m_format = DDSImage::Format::BC1;
		break;
	case FOURCC_DXT3:
		image->m_format = DDSImage::Format::BC2;
		break;
	case FOURCC_DXT5:
		image->m_format = DDSImage::Format::BC3;
		break;
	case FOURCC_BC4U:
		image->m_format = DDSImage::Format::BC4;
		break;
	case FOURCC_BC5U:
		image->m_format = DDSImage::Format::BC5;
		break;
	}

	if (has_header10)
	{
		switch (header10.dxgiFormat)
		{
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			image->m_format = DDSImage::Format::BC1;
			break;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			image->m_format = DDSImage::Format::BC2;
			break;
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			image->m_format = DDSImage::Format::BC3;
			break;
		case DXGI_FORMAT_BC4_UNORM:
			image->m_format = DDSImage::Format::BC4;
			break;
		case DXGI_FORMAT_BC5_UNORM:
			image->m_format = DDSImage::Format::BC5;
			break;
		case DXGI_FORMAT_BC6H_UF16:
			image->m_format = DDSImage::Format::BC6H;
			break;
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			image->m_format = DDSImage::Format::BC7;
			break;
		}
	}
	image->m_width = header.width;
	image->m_height = header.height;

	if (image->m_format == DDSImage::Format::BGRA)
	{
		_read_bgra(image, ptr, header, header10);
	}
	else
	{
		_read_blocks(image, ptr, header, header10);
	}
}

