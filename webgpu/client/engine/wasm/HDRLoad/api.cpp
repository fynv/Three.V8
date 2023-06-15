#include <memory.h>
#include <cmath>
#include <emscripten.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <gtc/packing.hpp>

extern "C"
{
	EMSCRIPTEN_KEEPALIVE void* alloc(unsigned size);
	EMSCRIPTEN_KEEPALIVE void dealloc(void* ptr);
	EMSCRIPTEN_KEEPALIVE void zero(void* ptr, unsigned size);
    EMSCRIPTEN_KEEPALIVE void* LoadMemory(unsigned char* data, unsigned size, int flip_x);
}


void* alloc(unsigned size)
{
	return malloc(size);
}

void dealloc(void* ptr)
{
	free(ptr);
}

void zero(void* ptr, unsigned size)
{
	memset(ptr, 0, size);
}

void g_flip_x(unsigned* data, int width, int height)
{
    unsigned* p_data = data;
    unsigned tmp;
    for (int y = 0; y < height; y++, p_data += width)
	{
		for (int x = 0; x < width / 2; x++)
		{
			tmp = p_data[x];
			p_data[x] = p_data[width - 1 - x];
			p_data[width - 1 - x] = tmp;
		}
	}
}

void* LoadMemory(unsigned char* data, unsigned size, int flip_x)
{
    int width, height, chn;
	float* rgb = stbi_loadf_from_memory(data, size, &width, &height, &chn, 3);

    unsigned count =  (unsigned)width*(unsigned)height;
    unsigned rgb9e5_size = count*4u;   

    unsigned char* output = (unsigned char*)malloc(sizeof(int)*2 + rgb9e5_size);

    int* p_int32 = (int*)output;
    p_int32[0] = width;
    p_int32[1] = height;

    unsigned* p_rgb9e5 = (unsigned*)(output + sizeof(int)*2);
    for (unsigned i=0; i<count; i++)
    {
        glm::vec3 col = {rgb[i*3], rgb[i*3+1], rgb[i*3+2]};
        p_rgb9e5[i] = packF3x9_E1x5(col);
    }

    stbi_image_free(rgb);

    if (flip_x!=0)
    {
        g_flip_x(p_rgb9e5, width, height);
    }

    return output;
}

