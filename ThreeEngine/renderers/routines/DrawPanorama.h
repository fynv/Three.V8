#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class DrawPanorama
{
public:
	DrawPanorama();

	void render(const GLBuffer* constant_camera, const GLTexture2D* tex_sky);

private:
	std::unique_ptr<GLProgram> m_prog;
};

