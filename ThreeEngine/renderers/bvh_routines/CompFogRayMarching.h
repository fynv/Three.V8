#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class Camera;
class Fog;
class CompFogRayMarching
{
public:
	CompFogRayMarching();

	struct RenderParams
	{
		const Fog* fog;

		const GLDynBuffer* constant_diretional_light;
		const GLDynBuffer* constant_diretional_shadow;
		unsigned tex_shadow;

		const BVHRenderTarget* target;
		const Camera* camera;
	};

	void render(const RenderParams& params);

private:
	void _render_no_shadow(const RenderParams& params);
	void _render_shadowed(const RenderParams& params);

	std::unique_ptr<GLProgram> m_prog;
	std::unique_ptr<GLProgram> m_prog_shadow;
};

