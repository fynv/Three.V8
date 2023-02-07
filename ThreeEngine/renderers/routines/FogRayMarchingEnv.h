#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class FogRayMarchingEnv
{
public:
	struct Options
	{
		bool msaa = false;
		bool has_probe_grid = false;
	};
	FogRayMarchingEnv(const Options& options);

	struct RenderParams
	{
		const GLTexture2D* tex_depth;
		const GLDynBuffer* constant_camera;
		const GLDynBuffer* constant_fog;
		const Lights* lights;		
	};

	void render(const RenderParams& params);


private:
	Options m_options;
	std::unique_ptr<GLProgram> m_prog;


};
