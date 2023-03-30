#include <GL/glew.h>
#include <gtx/hash.hpp>
#include <unordered_set>
#include "ModelComponents.h"
#include "utils/HDRImage.h"

template <typename T>
inline glm::ivec3 t_get_indices(T* indices, int face_id)
{
	int i0 = (int)indices[face_id * 3];
	int i1 = (int)indices[face_id * 3 + 1];
	int i2 = (int)indices[face_id * 3 + 2];
	return { i0, i1, i2 };
}

inline glm::ivec3 get_indices(void* indices, int type_indices, int face_id)
{
	if (type_indices == 1)
	{
		return t_get_indices((uint8_t*)indices, face_id);
	}
	else if (type_indices == 2)
	{
		return t_get_indices((uint16_t*)indices, face_id);
	}
	else if (type_indices == 4)
	{
		return t_get_indices((uint32_t*)indices, face_id);
	}
}

inline unsigned internalFormat(int type_indices)
{
	if (type_indices == 1)
	{
		return GL_R8UI;
	}
	else if (type_indices == 2)
	{
		return GL_R16UI;
	}
	else if (type_indices == 4)
	{
		return GL_R32UI;
	}

}

IndexTextureBuffer::IndexTextureBuffer(size_t size, int type_indices)
	: TextureBuffer(size, internalFormat(type_indices))
{

}

IndexTextureBuffer::~IndexTextureBuffer()
{

}

void Primitive::compute_wires()
{
	std::vector<glm::ivec2> wire_indices;
	{
		std::unordered_set<glm::ivec2> wire_index_set;

		if (index_buf != nullptr)
		{
			for (int j = 0; j < num_face; j++)
			{
				glm::ivec3 i_face = get_indices(cpu_indices->data(), type_indices, j);
				{
					glm::ivec2 i_wire = i_face.x > i_face.y ? glm::ivec2(i_face.y, i_face.x) : glm::ivec2(i_face.x, i_face.y);
					wire_index_set.insert(i_wire);
				}
				{
					glm::ivec2 i_wire = i_face.y > i_face.z ? glm::ivec2(i_face.z, i_face.y) : glm::ivec2(i_face.y, i_face.z);
					wire_index_set.insert(i_wire);
				}
				{
					glm::ivec2 i_wire = i_face.z > i_face.x ? glm::ivec2(i_face.x, i_face.z) : glm::ivec2(i_face.z, i_face.x);
					wire_index_set.insert(i_wire);
				}
			}
		}
		else
		{
			for (int j = 0; j < num_pos / 3; j++)
			{
				glm::ivec3 i_face = { j * 3, j * 3 + 1, j * 3 + 2 };
				{
					glm::ivec2 i_wire = glm::ivec2(i_face.x, i_face.y);
					wire_index_set.insert(i_wire);
				}
				{
					glm::ivec2 i_wire = glm::ivec2(i_face.y, i_face.z);
					wire_index_set.insert(i_wire);
				}
				{
					glm::ivec2 i_wire = glm::ivec2(i_face.x, i_face.z);
					wire_index_set.insert(i_wire);
				}
			}
		}
		auto iter = wire_index_set.begin();
		while (iter != wire_index_set.end())
		{
			wire_indices.push_back(*iter);
			iter++;
		}
	}
	num_wires = (int)wire_indices.size();
	wire_ind_buf = (Index)(new IndexTextureBuffer(sizeof(glm::ivec2) * num_wires, 4));
	wire_ind_buf->upload(wire_indices.data());
}


Lightmap::Lightmap(int width, int height, float texels_per_unit)
	:width(width), height(height), texels_per_unit(texels_per_unit)
{
	lightmap = std::unique_ptr<GLTexture2D>(new GLTexture2D);
	glBindTexture(GL_TEXTURE_2D, lightmap->tex_id);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Lightmap::Lightmap(const HDRImage& image)
	:width(image.width()), height(image.height())
{
	lightmap = std::unique_ptr<GLTexture2D>(new GLTexture2D);
	glBindTexture(GL_TEXTURE_2D, lightmap->tex_id);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB9_E5, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Lightmap::LoadImage(const HDRImage& image)
{
	glBindTexture(GL_TEXTURE_2D, lightmap->tex_id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, image.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Lightmap::GetImage(HDRImage& image)
{
	size_t buf_size = (size_t)width * (size_t)height * 3 * sizeof(float);

	glBindTexture(GL_TEXTURE_2D, lightmap->tex_id);
	image.m_width = width;
	image.m_height = height;
	free(image.m_buffer);
	image.m_buffer = (float*)malloc(buf_size);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, image.m_buffer);
	
	glBindTexture(GL_TEXTURE_2D, 0);
}

#if 0
ProbeVisibilityMap::ProbeVisibilityMap(int width, int height)
	:width(width), height(height)
{
	tex = std::unique_ptr<GLTexture2D>(new GLTexture2D);
	glBindTexture(GL_TEXTURE_2D, tex->tex_id);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8UI, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}
#endif

struct ModelConst
{
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
};

Mesh::Mesh()
{
	model_constant = std::unique_ptr<GLDynBuffer>(new GLDynBuffer(sizeof(ModelConst)));
}

