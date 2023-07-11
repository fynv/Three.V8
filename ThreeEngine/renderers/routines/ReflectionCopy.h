#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class Camera;
class ReflectionCopy
{
public:
	ReflectionCopy();

	void copy(unsigned tex_dst, unsigned tex_refl, int width, int height, const Camera* camera);

private:
	std::unique_ptr<GLProgram> m_prog;

};