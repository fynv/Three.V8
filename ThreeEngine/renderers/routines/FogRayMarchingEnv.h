#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class FogRayMarchingEnv
{
public:
	struct Options
	{
		Options()
		{
			memset(this, 0, sizeof(Options));
		}
		bool msaa = false;
		bool has_probe_grid = false;
		bool probe_reference_recorded = false;
		bool has_lod_probe_grid = false;
		bool is_reflect = false;
	};
	FogRayMarchingEnv(const Options& options);

	struct RenderParams
	{
		const GLTexture2D* tex_depth;
		const Camera* camera;
		const GLBuffer* constant_fog;
		const Lights* lights;		
	};

	void render(const RenderParams& params);


private:
	Options m_options;
	std::unique_ptr<GLProgram> m_prog;


};
