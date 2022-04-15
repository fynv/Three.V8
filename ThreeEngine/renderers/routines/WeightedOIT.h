#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class WeightedOIT
{
public:
	WeightedOIT();
	~WeightedOIT();

	void PreDraw(int width_video, int height_video, unsigned depth_rbo);
	void PostDraw();

private:
	int m_width_video = -1;
	int m_height_video = -1;
	unsigned m_tex_msaa0 = -1;
	unsigned m_tex_msaa1 = -1;
	unsigned m_fbo_msaa = -1;

	std::unique_ptr<GLShader> m_vert_shader;
	std::unique_ptr<GLShader> m_frag_shader;
	std::unique_ptr<GLProgram> m_prog;
};


