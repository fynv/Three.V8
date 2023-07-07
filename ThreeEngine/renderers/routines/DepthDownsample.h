#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class DepthDownsample
{
public:
	DepthDownsample();

	void render(unsigned tex_depth_4x);

private:
	std::unique_ptr<GLProgram> m_prog;

};