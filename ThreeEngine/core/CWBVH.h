#pragma once

#include <memory>
#include <vector>
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
class BVHRenderer;
class CWBVH
{
public:
	CWBVH(const Primitive* primitive, BVHRenderer* renderer);
	~CWBVH();
	
	BVHRenderer* m_renderer;
	Vec4TextureBuffer m_tex_bvh8;
	Vec4TextureBuffer m_tex_triangles;
	Int32TextureBuffer m_tex_indices;

	std::vector<std::unique_ptr<GLBuffer>> m_buf_level_indices;
};
