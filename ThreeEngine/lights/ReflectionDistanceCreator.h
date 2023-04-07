#pragma once

#include <memory>
#include "renderers/GLUtils.h"
#include "renderers/CubeRenderTarget.h"

class ReflectionDistanceCreator
{
public:
	ReflectionDistanceCreator(bool msaa = true);
	~ReflectionDistanceCreator();

	void Create(const CubeRenderTarget* target, ReflectionMap* reflection, float z_near, float z_far);

private:
	bool m_msaa;
	std::unique_ptr<GLProgram> m_prog_linearize;
	std::unique_ptr<GLProgram> m_prog_filter;

	unsigned m_tex_tmp0;

};
