#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "lights/IndirectLight.h"

class HemisphereLight : public IndirectLight
{
public:
	HemisphereLight();

	glm::vec3 skyColor = { 0.318f, 0.318f, 0.318f };
	glm::vec3 groundColor = { 0.01f, 0.025f, 0.025f };
	float intensity = 1.0f;

	GLBuffer m_constant;
	void updateConstant();

};
