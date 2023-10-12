#pragma once

#include <memory>
#include <string>
#include <vector>

#include "renderers/GLUtils.h"

class Primitive;
class RenderHeight
{
public:
	RenderHeight();

	struct RenderParams
	{		
		const GLBuffer* constant_height;
		const GLBuffer* constant_model;
		const Primitive* primitive;
	};

	void render(const RenderParams& params);

private:
	std::unique_ptr<GLProgram> m_prog;
};
