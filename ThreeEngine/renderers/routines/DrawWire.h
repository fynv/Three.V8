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
		glm::vec3 wire_color;
	};

	void render(const RenderParams& params);

private:
	std::unique_ptr<GLProgram> m_prog;

};

