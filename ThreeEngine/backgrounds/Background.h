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

