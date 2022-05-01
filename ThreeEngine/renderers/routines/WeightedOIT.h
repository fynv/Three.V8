#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class WeightedOIT
{
public:
	WeightedOIT(bool msaa);
	~WeightedOIT();

	void PreDraw(int width, int height, unsigned depth_rbo);
	void PostDraw();

private:
	bool m_msaa;
	int m_width = -1;
	int m_height = -1;
	unsigned m_tex0 = -1;
	unsigned m_tex1 = -1;
	unsigned m_fbo = -1;

	std::unique_ptr<GLShader> m_vert_shader;
	std::unique_ptr<GLShader> m_frag_shader;
	std::unique_ptr<GLProgram> m_prog;
};


