#pragma once

#include "core/Object3D.h"

class ProbeGridWidget : public Object3D
{
public:
	glm::vec3 coverage_min = { -10.0f, 0.0f, -10.0f };
	glm::vec3 coverage_max = { 10.0f, 10.0f, 10.0f };
	glm::ivec3 divisions = { 10, 5, 10 };
	float ypower = 1.0f;

};
