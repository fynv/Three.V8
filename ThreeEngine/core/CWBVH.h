#pragma once

#include <memory>
#include <glm.hpp>
#include "renderers/GLUtils.h"

class Int32TextureBuffer
{
public:
	std::unique_ptr<GLBuffer> buf;
	unsigned tex_id;

	Int32TextureBuffer();
	~Int32TextureBuffer();
	void upload(const int* data, size_t size);

};

class Vec4TextureBuffer
{
public:
	std::unique_ptr<GLBuffer> buf;
	unsigned tex_id;

	Vec4TextureBuffer();
	~Vec4TextureBuffer();
	void upload(const glm::vec4* data, size_t size);
};

class Primitive;
class CWBVH
{
public:
	CWBVH(const Primitive* primitive, const glm::mat4& model_matrix);
	~CWBVH();
	
	Vec4TextureBuffer m_tex_bvh8;
	Vec4TextureBuffer m_tex_triangles;
	Int32TextureBuffer m_tex_indices;
};
