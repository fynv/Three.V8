#pragma once

#include <memory>
#include <string>

#include "renderers/GLUtils.h"

class UpdateAABBs
{
public:
	UpdateAABBs(bool has_indices);

	void update(int num_nodes, GLBuffer* bvh_indices, GLBuffer* bvh_nodes, 
		GLBuffer* node_indices,	GLBuffer* prim_positions, unsigned tex_prim_indices);

private:
	bool m_has_indices;
	std::unique_ptr<GLProgram> m_prog;

};
