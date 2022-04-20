#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class DrawTexture
{
public:
	DrawTexture();

	void render(unsigned tex_id, int x, int y, int width, int height);

private:
	std::unique_ptr<GLShader> m_vert_shader;
	std::unique_ptr<GLShader> m_frag_shader;
	std::unique_ptr<GLProgram> m_prog;

};
