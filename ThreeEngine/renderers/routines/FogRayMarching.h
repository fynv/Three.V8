#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class Camera;
class Fog;
class FogRayMarching
{
public:
	FogRayMarching(bool msaa);

	struct RenderParams
	{
		const GLTexture2D* tex_depth;
		const Camera* camera;
		const Fog* fog;
		
		const GLBuffer* constant_diretional_light;
		const GLBuffer* constant_diretional_shadow;
		unsigned tex_shadow;

	};

	void render(const RenderParams& params);

private:
	void _render_no_shadow(const RenderParams& params);
	void _render_shadowed(const RenderParams& params);

	bool m_msaa;
	std::unique_ptr<GLProgram> m_prog;
	std::unique_ptr<GLProgram> m_prog_shadow;

};
