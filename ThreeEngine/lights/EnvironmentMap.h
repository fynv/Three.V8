#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"

class EnvironmentMap
{
public:
	EnvironmentMap();

	glm::vec4 shCoefficients[9];

	GLDynBuffer m_constant;
	void updateConstant();

};