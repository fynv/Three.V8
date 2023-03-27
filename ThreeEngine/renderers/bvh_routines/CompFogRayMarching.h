#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class Camera;
class Fog;
class ProbeRayList;
class LightmapRayList;

class CompFogRayMarching
{
public:
	CompFogRayMarching(int target_mode = 0);

	struct RenderParams
	{
		const Fog* fog;

		const GLDynBuffer* constant_diretional_light;
		const GLDynBuffer* constant_diretional_shadow;
		unsigned tex_shadow;

		const BVHRenderTarget* target;
		const Camera* camera;
		const ProbeRayList* prl;
		const LightmapRayList* lmrl;
	};

	void render(const RenderParams& params);

private:
	void _render_no_shadow(const RenderParams& params);
	void _render_shadowed(const RenderParams& params);

	int m_target_mode;
	std::unique_ptr<GLProgram> m_prog;
	std::unique_ptr<GLProgram> m_prog_shadow;
};

