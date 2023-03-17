#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class ProbeRayList;

class CompSkyBox
{
public:
	CompSkyBox(bool to_probe = false);

	void render(const GLDynBuffer* constant_camera, const GLCubemap* cubemap, const BVHRenderTarget* target);
	void render(const ProbeRayList* prl, const GLCubemap* cubemap, const BVHRenderTarget* target);

private:
	bool m_to_probe;
	std::unique_ptr<GLProgram> m_prog;
};

