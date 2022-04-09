#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "models/ModelComponents.h"

class MorphUpdate
{
public:
	MorphUpdate(bool has_tangent);

	struct Params
	{
		int num_targets;
		GLDynBuffer* buf_weights;
		const Primitive* primitive;
	};
	void update(const Params& params);

private:
	bool m_has_tangent;
	std::unique_ptr<GLShader> m_comp_shader;
	std::unique_ptr<GLProgram> m_prog;
};



