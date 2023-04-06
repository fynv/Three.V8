#pragma once

#include <memory>
#include "renderers/GLUtils.h"
#include "renderers/CubeRenderTarget.h"

class ReflectionDistanceCreator
{
public:
	ReflectionDistanceCreator();
	~ReflectionDistanceCreator();

	void Create(const CubeRenderTarget* target, ReflectionMap* reflection, float z_near, float z_far);

private:
	std::unique_ptr<GLProgram> m_prog;
};
