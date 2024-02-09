#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "lights/IndirectLight.h"

class EnvironmentMap : public IndirectLight
{
public:
	EnvironmentMap();
	~EnvironmentMap();

	glm::vec4 shCoefficients[9];
	float intensity = 1.0f;

	GLBuffer m_constant;
	void updateConstant();
	
};