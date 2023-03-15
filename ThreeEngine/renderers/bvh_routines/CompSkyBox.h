#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class CompSkyBox
{
public:
	CompSkyBox();

	void render(const GLDynBuffer* constant_camera, const GLCubemap* cubemap, const BVHRenderTarget* target);

private:
	std::unique_ptr<GLProgram> m_prog;
};

