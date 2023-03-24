#pragma once

#include <memory>
#include <vector>
#include <glm.hpp>
#include "renderers/GLUtils.h"

class ProbeGrid;
class LODProbeGrid;

class ProbeRayList
{
public:
	ProbeRayList(const glm::vec3& coverage_min, const glm::vec3& coverage_max, const glm::ivec3& divisions, int begin, int end, int num_directions = 256);
	ProbeRayList(const ProbeGrid& probe_grid, int begin, int end, int num_directions = 256);
	ProbeRayList(const LODProbeGrid& probe_grid, int begin, int end, int num_directions = 256);

	glm::mat4 rotation;
	int num_probes;
	int num_directions;

	GLDynBuffer m_constant;
	void updateConstant();

	std::vector<glm::vec4> positions;
	std::unique_ptr<GLBuffer> buf_positions;

	std::unique_ptr<GLBuffer> TexSHIrrWeight[9];

private:
	void _calc_shirr_weights();

};

