#pragma once


#include <memory>
#include "renderers/GLUtils.h"

class CopyDepth
{
public:
	CopyDepth();
	void render(unsigned tex_depth_in);

private:
	std::unique_ptr<GLProgram> m_prog;

};
