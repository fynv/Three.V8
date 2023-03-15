#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class CompDrawFog
{
public:
	struct Options
	{		
		bool has_environment_map = false;
		bool has_ambient_light = false;
		bool has_hemisphere_light = false;
	};
	CompDrawFog(const Options& options);

	struct RenderParams
	{		
		const GLDynBuffer* constant_fog;
		const Lights* lights;
		const BVHRenderTarget* target;
		const GLDynBuffer* constant_camera;
	};

	void render(const RenderParams& params);

private:
	Options m_options;
	std::unique_ptr<GLProgram> m_prog;


};

