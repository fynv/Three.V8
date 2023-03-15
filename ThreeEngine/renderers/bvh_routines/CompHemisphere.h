#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class CompHemisphere
{
public:
	CompHemisphere();

	void render(const GLDynBuffer* constant_camera, const GLDynBuffer* constant_hemisphere, const BVHRenderTarget* target);

private:
	std::unique_ptr<GLProgram> m_prog;
};


