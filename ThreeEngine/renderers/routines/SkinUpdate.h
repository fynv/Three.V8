#pragma once

#include "renderers/GLUtils.h"
#include "models/ModelComponents.h"


class SkinUpdate
{
public:
	SkinUpdate(bool has_tangent);

	struct Params
	{
		const Skin* skin;
		const Primitive* primitive;
	};
	void update(const Params& params);

private:	
	bool m_has_tangent;
	std::unique_ptr<GLShader> m_comp_shader;
	std::unique_ptr<GLProgram> m_prog;
};


