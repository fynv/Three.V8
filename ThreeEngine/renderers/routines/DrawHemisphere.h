#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class DrawHemisphere
{
public:
	DrawHemisphere();

	void render(const GLBuffer* constant_camera, const GLBuffer* constant_hemisphere);

private:
	std::unique_ptr<GLProgram> m_prog;
};

