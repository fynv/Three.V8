#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class ProbeRayList;
class LightmapRayList;
class CompHemisphere
{
public:
	CompHemisphere(int target_mode = 0);

	void render(const GLBuffer* constant_camera, const GLBuffer* constant_hemisphere, const BVHRenderTarget* target);
	void render(const ProbeRayList* prl, const GLBuffer* constant_hemisphere, const BVHRenderTarget* target);
	void render(const LightmapRayList* lmrl, const GLBuffer* constant_hemisphere, const BVHRenderTarget* target);

private:
	int m_target_mode;
	std::unique_ptr<GLProgram> m_prog;
};


