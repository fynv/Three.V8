#pragma once

#include <memory>
#include <string>

#include "renderers/GLUtils.h"

class BVHRenderTarget;
class LightmapRayList;
class Lightmap;

class LightmapUpdate
{
public:
	LightmapUpdate();

	struct RenderParams
	{
		float mix_rate;
		const BVHRenderTarget* source;
		const LightmapRayList* lmrl;
		const Lightmap* target;
	};

	void update(const RenderParams& params);

private:
	std::unique_ptr<GLProgram> m_prog;

};
