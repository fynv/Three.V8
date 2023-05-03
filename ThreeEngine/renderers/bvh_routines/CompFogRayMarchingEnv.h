#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class ProbeRayList;
class LightmapRayList;

class CompFogRayMarchingEnv
{
public:
	struct Options
	{		
		Options()
		{
			memset(this, 0, sizeof(Options));
		}
		int target_mode = 0;
		bool has_probe_grid = false;
		bool probe_reference_recorded = false;
		bool has_lod_probe_grid = false;
	};
	CompFogRayMarchingEnv(const Options& options);

	struct RenderParams
	{		
		const GLBuffer* constant_fog;
		const Lights* lights;
		
		const BVHRenderTarget* target;
		const GLBuffer* constant_camera;
		const ProbeRayList* prl;
		const LightmapRayList* lmrl;
	};

	void render(const RenderParams& params);


private:
	Options m_options;
	std::unique_ptr<GLProgram> m_prog;

};

