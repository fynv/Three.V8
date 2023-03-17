#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class ProbeRayList;

class CompHemisphere
{
public:
	CompHemisphere(bool to_probe = false);

	void render(const GLDynBuffer* constant_camera, const GLDynBuffer* constant_hemisphere, const BVHRenderTarget* target);
	void render(const ProbeRayList* prl, const GLDynBuffer* constant_hemisphere, const BVHRenderTarget* target);

private:
	bool m_to_probe;
	std::unique_ptr<GLProgram> m_prog;
};


