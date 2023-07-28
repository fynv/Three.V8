#pragma once

#include <memory>
#include <string>

#include "renderers/GLUtils.h"

class UpdateTriangles
{
public:
	UpdateTriangles(bool has_indices);

	void update(int num_triangles, GLBuffer* bvh_indices, GLBuffer* bvh_triangles,
		GLBuffer* prim_positions, unsigned tex_prim_indices);

private:
	bool m_has_indices;
	std::unique_ptr<GLProgram> m_prog;

};
