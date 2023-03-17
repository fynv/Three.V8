#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class ProbeRayList;

class CompFogRayMarchingEnv
{
public:
	struct Options
	{		
		bool to_probe = false;
		bool has_probe_grid = false;
		bool probe_reference_recorded = false;
		bool has_lod_probe_grid = false;
	};
	CompFogRayMarchingEnv(const Options& options);

	struct RenderParams
	{		
		const GLDynBuffer* constant_fog;
		const Lights* lights;
		
		const BVHRenderTarget* target;
		const GLDynBuffer* constant_camera;
		const ProbeRayList* prl;
	};

	void render(const RenderParams& params);


private:
	Options m_options;
	std::unique_ptr<GLProgram> m_prog;

};

