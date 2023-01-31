#pragma once

#include <vector>
#include <memory>
#include "renderers/GLUtils.h"

class Object3D;
class GLPickingTarget
{
public:
	GLPickingTarget();
	~GLPickingTarget();

	int m_width = -1;
	int m_height = -1;

	std::unique_ptr<GLTexture2D> m_tex_idx;
	std::unique_ptr<GLTexture2D> m_tex_depth;

	unsigned m_fbo = -1;
	bool update_framebuffers(int width, int height);

	void bind_buffer();

	struct IdxInfo
	{
		Object3D* obj;
		int primitive_idx;
	};

	std::vector<IdxInfo> m_idx_info;

	const IdxInfo& pick_obj(int x, int y);
};

