#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"

class Fog
{
public:
	Fog();

	glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
	float density = 0.1f;
	int max_num_steps = 50;
	float min_step = 0.15f;

	GLBuffer m_constant;
	void updateConstant();
};

