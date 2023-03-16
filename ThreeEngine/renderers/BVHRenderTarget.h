#pragma once

#include <memory>
#include "renderers/GLUtils.h"
#include "renderers/bvh_routines/CompWeightedOIT.h"

class BVHRenderTarget
{
public:
	BVHRenderTarget();
	~BVHRenderTarget();

	int m_width = -1;
	int m_height = -1;

	std::unique_ptr<GLTexture2D> m_tex_video;
	std::unique_ptr<GLTexture2D> m_tex_depth;

	bool update(int width, int height, bool color = true, bool depth = true);

	CompWeightedOIT::Buffers m_OITBuffers;
	void update_oit_buffers();
};



