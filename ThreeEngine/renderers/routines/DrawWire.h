#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class Primitive;
class DrawWire
{
public:
	DrawWire();

	struct RenderParams
	{	
		const GLDynBuffer* constant_camera;
		const GLDynBuffer* constant_model;
		const Primitive* primitive;
		float radius;
	};

	void render(const RenderParams& params);

private:
	std::unique_ptr<GLShader> m_vert_shader;
	std::unique_ptr<GLShader> m_geo_shader;
	std::unique_ptr<GLShader> m_frag_shader;
	std::unique_ptr<GLProgram> m_prog;

};

