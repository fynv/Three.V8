#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class ProbeRayList;
class LightmapRayList;
class CompSkyBox
{
public:
	CompSkyBox(int target_mode = 0);

	void render(const GLDynBuffer* constant_camera, const GLCubemap* cubemap, const BVHRenderTarget* target);
	void render(const ProbeRayList* prl, const GLCubemap* cubemap, const BVHRenderTarget* target);
	void render(const LightmapRayList* lmrl, const GLCubemap* cubemap, const BVHRenderTarget* target);

private:
	int m_target_mode;
	std::unique_ptr<GLProgram> m_prog;
};

