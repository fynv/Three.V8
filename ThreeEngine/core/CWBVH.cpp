#include <GL/glew.h>
#include "CWBVH.h"
#include "models/ModelComponents.h"
#include "BVH.h"
#include "BVH8Converter.h"

Int32TextureBuffer::Int32TextureBuffer()
{
	glGenTextures(1, &tex_id);
}

Int32TextureBuffer::~Int32TextureBuffer()
{
	glDeleteTextures(1, &tex_id);
}

void Int32TextureBuffer::upload(const int* data, size_t size)
{
	buf = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(int) * size, GL_TEXTURE_BUFFER));
	buf->upload(data);
	glBindTexture(GL_TEXTURE_BUFFER, tex_id);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, buf->m_id);
}

Vec4TextureBuffer::Vec4TextureBuffer()
{
	glGenTextures(1, &tex_id);
}

Vec4TextureBuffer::~Vec4TextureBuffer()
{
	glDeleteTextures(1, &tex_id);
}

void Vec4TextureBuffer::upload(const glm::vec4* data, size_t size)
{
	buf = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(glm::vec4) * size, GL_TEXTURE_BUFFER));
	buf->upload(data);
	glBindTexture(GL_TEXTURE_BUFFER, tex_id);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buf->m_id);
}


template <typename T>
inline void t_get_indices(T* indices, int face_id, unsigned& i0, unsigned& i1, unsigned& i2)
{
	i0 = indices[face_id * 3];
	i1 = indices[face_id * 3 + 1];
	i2 = indices[face_id * 3 + 2];
}

inline void get_indices(void* indices, int type_indices, int face_id, unsigned& i0, unsigned& i1, unsigned& i2)
{
	if (type_indices == 1)
	{
		t_get_indices((uint8_t*)indices, face_id, i0, i1, i2);
	}
	else if (type_indices == 2)
	{
		t_get_indices((uint16_t*)indices, face_id, i0, i1, i2);
	}
	else if (type_indices == 4)
	{
		t_get_indices((uint32_t*)indices, face_id, i0, i1, i2);
	}
}

CWBVH::CWBVH(const Primitive* primitive, const glm::mat4& model_matrix)
{	
	std::vector<flex_bvh::Triangle> triangles;

	if (primitive->index_buf != nullptr)
	{
		for (int i = 0; i < primitive->num_face; i++)
		{
			unsigned i0, i1, i2;
			get_indices(primitive->cpu_indices->data(), primitive->type_indices, i, i0, i1, i2);

			glm::vec4 v0, v1, v2;
			v0 = primitive->cpu_pos->data()[i0];
			v1 = primitive->cpu_pos->data()[i1];
			v2 = primitive->cpu_pos->data()[i2];

			v0 = model_matrix * v0;
			v1 = model_matrix * v1;
			v2 = model_matrix * v2;

			triangles.emplace_back(flex_bvh::Triangle(v0, v1, v2));
		}
	}
	else
	{
		for (int i = 0; i < primitive->num_pos / 3; i++)
		{
			glm::vec4 v0, v1, v2;
			v0 = primitive->cpu_pos->data()[i * 3];
			v1 = primitive->cpu_pos->data()[i * 3 + 1];
			v2 = primitive->cpu_pos->data()[i * 3 + 2];

			v0 = model_matrix * v0;
			v1 = model_matrix * v1;
			v2 = model_matrix * v2;

			triangles.emplace_back(flex_bvh::Triangle(v0, v1, v2));
		}
	}

	flex_bvh::BVH2 bvh2;
	bvh2.create_from_triangles(triangles);

	flex_bvh::BVH8 bvh8;
	flex_bvh::ConvertBVH2ToBVH8(bvh2, bvh8);

	m_tex_bvh8.upload((glm::vec4*)bvh8.nodes.data(), bvh8.nodes.size() * 5);

	std::vector<glm::vec4> mapped_triangles(bvh8.indices.size() * 3);
	for (size_t i = 0; i < bvh8.indices.size(); i++)
	{
		int index = bvh8.indices[i];
		const flex_bvh::Triangle& tri = triangles[index];
		mapped_triangles[i * 3] = glm::vec4(tri.position_0, 1.0f);
		mapped_triangles[i * 3 + 1] = glm::vec4(tri.position_1 - tri.position_0, 0.0f);
		mapped_triangles[i * 3 + 2] = glm::vec4(tri.position_2 - tri.position_0, 0.0f);
	}

	m_tex_triangles.upload(mapped_triangles.data(), mapped_triangles.size());
	m_tex_indices.upload(bvh8.indices.data(), bvh8.indices.size());
}

CWBVH::~CWBVH()
{

}

