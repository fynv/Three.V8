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
	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
};

#include "renderers/GLUtils.h"

class HemisphereBackground : public Background
{
public:
	HemisphereBackground();

	glm::vec3 skyColor = { 0.318f, 0.318f, 0.318f };
	glm::vec3 groundColor = { 0.01f, 0.025f, 0.025f };

	GLDynBuffer m_constant;
	void updateConstant();
};

class CubeBackground : public Background
{
public:
	GLCubemap cubemap;
};

