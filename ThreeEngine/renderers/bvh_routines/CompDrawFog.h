#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class ReflectionRenderTarget;
class CompDrawFog
{
public:
	struct Options
	{		
		Options()
		{
			memset(this, 0, sizeof(Options));
		}
		int target_mode = 0;
		bool has_environment_map = false;
		bool has_ambient_light = false;
		bool has_hemisphere_light = false;
	};
	CompDrawFog(const Options& options);

	struct RenderParams
	{		
		const GLBuffer* constant_fog;
		const Lights* lights;
		const BVHRenderTarget* target;		
		const ReflectionRenderTarget* normal_depth;
	};

	void render(const RenderParams& params);

private:
	Options m_options;
	std::unique_ptr<GLProgram> m_prog;


};

