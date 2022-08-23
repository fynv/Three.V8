#pragma once

#include <memory>
#include "renderers/routines/WeightedOIT.h"

class GLRenderTarget
{
public:
	GLRenderTarget(bool default_buffer, bool msaa);
	~GLRenderTarget();

	bool msaa() const
	{
		return m_fbo_msaa != (unsigned)(-1);
	}

	int m_width = -1;
	int m_height = -1;
	
	unsigned m_tex_video = -1;
	unsigned m_rbo_video = -1;
	unsigned m_fbo_video = 0;

	unsigned m_tex_msaa = -1;
	unsigned m_rbo_msaa = -1;
	unsigned m_fbo_msaa = -1;
	bool update_framebuffers(int width, int height);

	WeightedOIT::Buffers m_OITBuffers;
	void update_oit_buffers();

	void bind_buffer();
	void resolve_msaa();
	void blit_buffer(int width_wnd, int height_wnd, int margin);

};
