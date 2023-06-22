#include <memory.h>
#include <cmath>
#include <emscripten.h>
#include <gtc/packing.hpp>
#include <vector>

extern "C"
{
	EMSCRIPTEN_KEEPALIVE void* alloc(unsigned size);
	EMSCRIPTEN_KEEPALIVE void dealloc(void* ptr);
	EMSCRIPTEN_KEEPALIVE void zero(void* ptr, unsigned size); 
    EMSCRIPTEN_KEEPALIVE void presample_irradiance(int num_probes, int irr_res, const void* p_probe_data, void* p_u32, int stride, int offset);
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


inline glm::vec2 signNotZero(const glm::vec2& v)
{
	return glm::vec2((v.x >= 0.0f) ? 1.0f : -1.0f, (v.y >= 0.0f) ? 1.0f : -1.0f);
}

inline glm::vec3 oct_to_vec3(const glm::vec2& e)
{
	glm::vec3 v = glm::vec3(glm::vec2(e.x, e.y), 1.0f - fabsf(e.x) - fabsf(e.y));
	if (v.z < 0.0f)
	{
		glm::vec2 tmp = (1.0f - glm::abs(glm::vec2(v.y, v.x))) * signNotZero(glm::vec2(v.x, v.y));
		v.x = tmp.x;
		v.y = tmp.y;
	}
	return glm::normalize(v);
}

inline void PresampleSH(const glm::vec4* shCoefficients, glm::vec3* tex_data, int res)
{
	for (int y = 0; y < res; y++)
	{
		for (int x = 0; x < res; x++)
		{
			glm::vec2 v2 = glm::vec2(float(x) + 0.5f, float(y) + 0.5f) / float(res) * 2.0f - 1.0f;
			glm::vec3 dir = oct_to_vec3(v2);
			
			// band 0
			glm::vec4 irr = shCoefficients[0] * 0.886227f;

			// band 1
			irr += shCoefficients[1] * 2.0f * 0.511664f * dir.y;
			irr += shCoefficients[2] * 2.0f * 0.511664f * dir.z;
			irr += shCoefficients[3] * 2.0f * 0.511664f * dir.x;

			// band 2
			irr += shCoefficients[4] * 2.0f * 0.429043f * dir.x * dir.y;
			irr += shCoefficients[5] * 2.0f * 0.429043f * dir.y * dir.z;
			irr += shCoefficients[6] * (0.743125f * dir.z * dir.z - 0.247708f);
			irr += shCoefficients[7] * 2.0f * 0.429043f * dir.x * dir.z;
			irr += shCoefficients[8] * 0.429043f * (dir.x * dir.x - dir.y * dir.y);

			tex_data[x + y * res] = irr;
		}
	}
}

inline unsigned packU32(const glm::vec3& rgb)
{
	glm::vec3 col = glm::clamp(rgb, glm::vec3(0.0), glm::vec3(65000.0f));	
	return packF3x9_E1x5(col);
}

void presample_irradiance(int num_probes, int irr_res, const void* p_probe_data, void* p_u32, int stride, int offset)
{
    int pack_size = int(ceilf(sqrtf(float(num_probes))));
    int irr_pack_res = pack_size * (irr_res + 2);    

    const glm::vec4* probe_data = (const glm::vec4*)p_probe_data;
    unsigned* u32 = (unsigned*)p_u32;
	memset(u32, 0, sizeof(unsigned)* irr_pack_res * irr_pack_res);

    for (int index = 0; index < num_probes; index++)
	{
        std::vector<glm::vec3> tex_data(irr_res * irr_res);
        PresampleSH(probe_data + index*stride + offset, tex_data.data(), irr_res);
        for (int y = 0; y < irr_res; y++)
		{
			for (int x = 0; x < irr_res; x++)
			{
				glm::vec3 irr = tex_data[x + y * irr_res];
				int out_x = (index % pack_size) * (irr_res + 2) + x + 1;
				int out_y = (index / pack_size) * (irr_res + 2) + y + 1;
				u32[out_x + out_y * irr_pack_res] =  packU32(irr);
			}
		}

        {
			glm::vec3 irr = tex_data[(irr_res - 1) + (irr_res - 1) * irr_res];
			int out_x = (index % pack_size) * (irr_res + 2);
			int out_y = (index / pack_size) * (irr_res + 2);
			u32[out_x + out_y * irr_pack_res] = packU32(irr);
		}
		{
			glm::vec3 irr = tex_data[(irr_res - 1) * irr_res];
			int out_x = (index % pack_size) * (irr_res + 2) + irr_res + 1;
			int out_y = (index / pack_size) * (irr_res + 2);
			u32[out_x + out_y * irr_pack_res] = packU32(irr);
		}

		{
			glm::vec3 irr = tex_data[irr_res - 1];
			int out_x = (index % pack_size) * (irr_res + 2);
			int out_y = (index / pack_size) * (irr_res + 2) + irr_res + 1;
			u32[out_x + out_y * irr_pack_res] = packU32(irr);
		}

		{
			glm::vec3 irr = tex_data[0];
			int out_x = (index % pack_size) * (irr_res + 2) + irr_res + 1;
			int out_y = (index / pack_size) * (irr_res + 2) + irr_res + 1;
			u32[out_x + out_y * irr_pack_res] = packU32(irr);
		}

        for (int x = 0; x < irr_res; x++)
		{
			{
				glm::vec3 irr = tex_data[irr_res - 1 - x];
				int out_x = (index % pack_size) * (irr_res + 2) + x + 1;
				int out_y = (index / pack_size) * (irr_res + 2);
				u32[out_x + out_y * irr_pack_res] = packU32(irr);
			}
			{
				glm::vec3 irr = tex_data[(irr_res - 1 - x) + (irr_res - 1) * irr_res];
				int out_x = (index % pack_size) * (irr_res + 2) + x + 1;
				int out_y = (index / pack_size) * (irr_res + 2) + irr_res + 1;
				u32[out_x + out_y * irr_pack_res] = packU32(irr);
			}
		}
		for (int y = 0; y < irr_res; y++)
		{
			{
				glm::vec3 irr = tex_data[(irr_res - 1 - y) * irr_res];
				int out_x = (index % pack_size) * (irr_res + 2);
				int out_y = (index / pack_size) * (irr_res + 2) + y + 1;
				u32[out_x + out_y * irr_pack_res] = packU32(irr);
			}

			{
				glm::vec3 irr = tex_data[(irr_res - 1) + (irr_res - 1 - y) * irr_res];
				int out_x = (index % pack_size) * (irr_res + 2) + irr_res + 1;
				int out_y = (index / pack_size) * (irr_res + 2) + y + 1;
				u32[out_x + out_y * irr_pack_res] = packU32(irr);
			}
		}

    }

}

