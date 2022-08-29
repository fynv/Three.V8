#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class DrawHemisphere
{
public:
	DrawHemisphere();

	void render(const GLDynBuffer* constant_camera, const GLDynBuffer* constant_hemisphere);

private:
	std::unique_ptr<GLProgram> m_prog;
};

