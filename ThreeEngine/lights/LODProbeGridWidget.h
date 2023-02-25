#pragma once

#include "core/Object3D.h"

class LODProbeGrid;
class LODProbeGridWidget : public Object3D
{
public:
	glm::vec3 coverage_min = { -10.0f, 0.0f, -10.0f };
	glm::vec3 coverage_max = { 10.0f, 10.0f, 10.0f };
	glm::ivec3 base_divisions = { 10, 5, 10 };	

	LODProbeGrid* probe_grid = nullptr;
};
