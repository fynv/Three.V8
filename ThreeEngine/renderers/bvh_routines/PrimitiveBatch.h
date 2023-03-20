#pragma once

#include "renderers/GLUtils.h"
#include "models/ModelComponents.h"


class PrimitiveBatch
{
public:
	struct Options
	{
		Options()
		{
			memset(this, 0, sizeof(Options));
		}
		bool has_color = false;
		bool has_uv = false;
		bool has_tangent = false;
	};

	PrimitiveBatch(const Options& options);

	struct Params
	{
		int offset;
		const GLDynBuffer* constant_model;
		const Primitive* primitive_in;
		const Primitive* primitive_out;
	};
	void update(const Params& params);

private:
	Options m_options;
	std::unique_ptr<GLProgram> m_prog;
};


