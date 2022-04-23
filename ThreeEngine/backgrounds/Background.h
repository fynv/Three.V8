#pragma once

class Background
{
public:
	Background(){}
	virtual ~Background(){}
};

#include <glm.hpp>

class ColorBackground : public Background
{
public:
	glm::vec3 color;
};

#include "renderers/GLUtils.h"

class CubeBackground : public Background
{
public:
	int width, height;
	GLCubemap cubemap;
};

