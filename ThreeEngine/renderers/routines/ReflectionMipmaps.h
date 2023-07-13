#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class ReflectionMipmaps
{
public:
	ReflectionMipmaps();

	void downsample(unsigned tex, int level, int width, int height);

private:
	std::unique_ptr<GLProgram> m_prog;

};



