#pragma once

#include <memory>
#include "renderers/GLUtils.h"


class BVHRenderTarget;
class ReflectionRenderTarget;
class CompColorBg
{
public:
	CompColorBg();

	void render(const ReflectionRenderTarget* normal_depth, const glm::vec3& color, const BVHRenderTarget* target);
	

private:	
	std::unique_ptr<GLProgram> m_prog;
};


