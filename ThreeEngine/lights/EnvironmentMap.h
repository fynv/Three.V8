#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"

class EnvironmentMap
{
public:
	EnvironmentMap();
	~EnvironmentMap();

	glm::vec4 shCoefficients[9];

	GLDynBuffer m_constant;
	void updateConstant();

	unsigned id_cube_reflection; // 128*128*7

};