#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "lights/IndirectLight.h"

class HemisphereLight : public IndirectLight
{
public:
	HemisphereLight();

	glm::vec3 skyColor = { 1.0f, 1.0f, 1.0f };
	glm::vec3 groundColor = { 1.0f, 1.0f, 1.0f };
	float intensity = 1.0f;

	GLDynBuffer m_constant;
	void updateConstant();

};
