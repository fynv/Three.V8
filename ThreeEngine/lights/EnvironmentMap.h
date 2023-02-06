#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "lights/IndirectLight.h"

class EnvironmentMap : public IndirectLight
{
public:
	EnvironmentMap();
	~EnvironmentMap();

	bool allocated = false;
	void allocate();

	glm::vec4 shCoefficients[9];

	GLDynBuffer m_constant;
	void updateConstant();

	unsigned id_cube_reflection; // 128*128*7	
};