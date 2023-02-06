#pragma once

#include <vector>
#include <memory>
#include "renderers/GLUtils.h"

class GLSpaceProbeTarget
{
public:
	GLSpaceProbeTarget();
	~GLSpaceProbeTarget();

	int m_width = -1;
	int m_height = -1;
	
	std::unique_ptr<GLTexture2D> m_tex_depth;

	unsigned m_fbo = -1;
	bool update_framebuffers(int width, int height);

	void bind_buffer();
	
};

