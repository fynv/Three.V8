#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "lights/IndirectLight.h"

class AmbientLight : public IndirectLight
{
public:
	AmbientLight();

	glm::vec3 color;
	float intensity;

	GLDynBuffer m_constant;
	void updateConstant();

};
