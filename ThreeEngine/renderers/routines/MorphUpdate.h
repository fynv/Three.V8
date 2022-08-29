#pragma once

#include "renderers/GLUtils.h"
#include "models/ModelComponents.h"

class MorphUpdate
{
public:
	MorphUpdate(bool has_tangent, bool sparse);

	struct Params
	{
		GLDynBuffer* buf_weights;
		const Primitive* primitive;
	};
	void update(const Params& params);

private:
	bool m_has_tangent;
	bool m_sparse;	
	std::unique_ptr<GLProgram> m_prog;
};



