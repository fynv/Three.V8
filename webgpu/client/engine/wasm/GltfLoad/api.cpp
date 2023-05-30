#include <glm.hpp>
#include <cstdio>
#include <vector>
#include <memory.h>
#include <emscripten.h>

extern "C"
{
	EMSCRIPTEN_KEEPALIVE void* alloc(unsigned size);
	EMSCRIPTEN_KEEPALIVE void dealloc(void* ptr);
	EMSCRIPTEN_KEEPALIVE void zero(void* ptr, unsigned size);
    EMSCRIPTEN_KEEPALIVE void vec3_to_vec4(const void* ptr_vec3, void* ptr_vec4, int count, float w);
    EMSCRIPTEN_KEEPALIVE void calc_normal(int num_face, int num_pos, int type_indices, const void* p_indices, const void* ptr_pos, void* ptr_norm);
	EMSCRIPTEN_KEEPALIVE void calc_tangent(int num_face, int num_pos, int type_indices, const void* p_indices, const void* ptr_pos, const void* ptr_uv, void* ptr_tangent, void* ptr_bitangent);
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

void vec3_to_vec4(const void* ptr_vec3, void* ptr_vec4, int count, float w)
{
    const glm::vec3* p_in = (const glm::vec3*)ptr_vec3;
    glm::vec4* p_out = (glm::vec4*)ptr_vec4;
    for (int i=0; i<count; i++)
    {
        p_out[i] = glm::vec4(p_in[i], w);
    }
}


template<typename T>
inline void t_calc_normal(int num_face, int num_pos, const T* p_indices, const glm::vec3* p_pos, glm::vec4* p_norm)
{	
	std::vector<float> counts(num_pos, 0.0f);
	for (int j = 0; j < num_face; j++)
	{
		glm::uvec3 ind;
		if (p_indices != nullptr)
		{
			ind.x = (uint32_t)p_indices[j * 3];
			ind.y = (uint32_t)p_indices[j * 3 + 1];
			ind.z = (uint32_t)p_indices[j * 3 + 2];
		}
		else
		{
			ind.x = j * 3;
			ind.y = j * 3 + 1;
			ind.z = j * 3 + 2;
		}

		glm::vec3 v0 = p_pos[ind.x];
		glm::vec3 v1 = p_pos[ind.y];
		glm::vec3 v2 = p_pos[ind.z];
		glm::vec4 face_normals = glm::vec4(glm::cross(v1 - v0, v2 - v0), 0.0f);

		p_norm[ind.x] += face_normals;
		p_norm[ind.y] += face_normals;
		p_norm[ind.z] += face_normals;
		counts[ind.x] += 1.0f;
		counts[ind.y] += 1.0f;
		counts[ind.z] += 1.0f;
	}

	for (int j = 0; j < num_pos; j++)
		p_norm[j] = p_norm[j] / counts[j];
}

void calc_normal(int num_face, int num_pos, int type_indices, const void* p_indices, const void* ptr_pos, void* ptr_norm)
{
    const glm::vec3* p_pos = (const glm::vec3*)ptr_pos;
    glm::vec4* p_norm = (glm::vec4*)ptr_norm;

    if (type_indices == 1)
	{
		t_calc_normal<uint8_t>(num_face, num_pos, (uint8_t*)p_indices, p_pos, p_norm);
	}
	if (type_indices == 2)
	{
		t_calc_normal<uint16_t>(num_face, num_pos, (uint16_t*)p_indices, p_pos, p_norm);
	}
	else if (type_indices == 4)
	{
		t_calc_normal<uint32_t>(num_face, num_pos, (uint32_t*)p_indices, p_pos, p_norm);
	}
}


template<typename T>
inline void t_calc_tangent(int num_face, int num_pos, const T* p_indices, const glm::vec3* p_pos, const glm::vec2* p_uv, glm::vec4* p_tangent, glm::vec4* p_bitangent)
{
	std::vector<float> counts(num_pos, 0.0f);
	for (int j = 0; j < num_face; j++)
	{
		glm::uvec3 ind;
		if (p_indices != nullptr)
		{
			ind.x = (uint32_t)p_indices[j * 3];
			ind.y = (uint32_t)p_indices[j * 3 + 1];
			ind.z = (uint32_t)p_indices[j * 3 + 2];
		}
		else
		{
			ind.x = j * 3;
			ind.y = j * 3 + 1;
			ind.z = j * 3 + 2;
		}

		glm::vec3 v0 = p_pos[ind.x];
		glm::vec3 v1 = p_pos[ind.y];
		glm::vec3 v2 = p_pos[ind.z];
		glm::vec2 texCoord0 = p_uv[ind.x];
		glm::vec2 texCoord1 = p_uv[ind.y];
		glm::vec2 texCoord2 = p_uv[ind.z];

		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;
		glm::vec2 delta1 = texCoord1 - texCoord0;
		glm::vec2 delta2 = texCoord2 - texCoord0;

		float f = 1.0f / (delta1[0] * delta2[1] - delta2[0] * delta1[1]);
		glm::vec4 tagent = glm::vec4((f * delta2[1]) * edge1 - (f * delta1[1]) * edge2, 0.0f);
		glm::vec4 bitangent = glm::vec4((-f * delta2[0]) * edge1 + (f * delta1[0]) * edge2, 0.0f);

		p_tangent[ind.x] += tagent;
		p_tangent[ind.y] += tagent;
		p_tangent[ind.z] += tagent;

		p_bitangent[ind.x] += bitangent;
		p_bitangent[ind.y] += bitangent;
		p_bitangent[ind.z] += bitangent;

		counts[ind.x] += 1.0f;
		counts[ind.y] += 1.0f;
		counts[ind.z] += 1.0f;
	}

	for (int j = 0; j < num_pos; j++)
	{
		p_tangent[j] = p_tangent[j] / counts[j];
		p_bitangent[j] = p_bitangent[j] / counts[j];
	}
}

void calc_tangent(int num_face, int num_pos, int type_indices, const void* p_indices, const void* ptr_pos, const void* ptr_uv, void* ptr_tangent, void* ptr_bitangent)
{
	const glm::vec3* p_pos = (const glm::vec3*)ptr_pos;
	const glm::vec2* p_uv  = (const glm::vec2*)ptr_uv;
    glm::vec4* p_tangent = (glm::vec4*)ptr_tangent;
	glm::vec4* p_bitangent = (glm::vec4*)ptr_bitangent;

	if (type_indices == 1)
	{
		t_calc_tangent<uint8_t>(num_face, num_pos, (uint8_t*)p_indices, p_pos, p_uv, p_tangent, p_bitangent);
	}
	else if (type_indices == 2)
	{
		t_calc_tangent<uint16_t>(num_face, num_pos, (uint16_t*)p_indices, p_pos, p_uv, p_tangent, p_bitangent);
	}
	else if (type_indices == 4)
	{
		t_calc_tangent<uint32_t>(num_face, num_pos, (uint32_t*)p_indices, p_pos, p_uv, p_tangent, p_bitangent);
	}

}


