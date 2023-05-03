#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class DrawSkyBox
{
public:
	DrawSkyBox();

	void render(const GLBuffer* constant_camera, const GLCubemap* cubemap);

private:
	std::unique_ptr<GLProgram> m_prog;
};

