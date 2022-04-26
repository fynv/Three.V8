#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "lights/IndirectLight.h"

class AmbientLight : public IndirectLight
{
public:
	AmbientLight();

	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	float intensity = 0.2f;

	GLDynBuffer m_constant;
	void updateConstant();

};