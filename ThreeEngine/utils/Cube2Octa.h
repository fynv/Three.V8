#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class Cube2Octa
{
public:
	Cube2Octa();
	void convert(const GLCubemap* cube, GLTexture2D* octa, int width, int height);


private:
	std::unique_ptr<GLProgram> m_prog;

};