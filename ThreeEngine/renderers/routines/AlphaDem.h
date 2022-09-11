#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class AlphaDem
{
public:
	AlphaDem();
	void render(unsigned tex_id, int x, int y, int width, int height);

private:
	std::unique_ptr<GLProgram> m_prog;

};
