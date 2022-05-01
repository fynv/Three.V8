#pragma once

#include <memory>
#include "renderers/routines/WeightedOIT.h"

class GLRenderTarget
{
public:
	GLRenderTarget(bool default_buffer, bool msaa);
	~GLRenderTarget();

	int m_width = -1;
	int m_height = -1;
	
	unsigned m_tex_video = -1;
	unsigned m_rbo_video = -1;
	unsigned m_fbo_video = 0;

	unsigned m_tex_msaa = -1;
	unsigned m_rbo_msaa = -1;
	unsigned m_fbo_msaa = -1;
	void update_framebuffers(int width, int height);

	std::unique_ptr<WeightedOIT> OITResolver;

	void render_begin();
	
	void transparent_begin();
	void transparent_end();

	void resolve_msaa();

	void blit_buffer(int width_wnd, int height_wnd, int margin);

};
