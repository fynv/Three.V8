#pragma once

#include <vector>
#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "lights/IndirectLight.h"

class ProbeGrid : public IndirectLight
{
public:
	ProbeGrid();
	~ProbeGrid();

	glm::vec3 coverage_min = { -10.0f, 0.0f, -10.0f };
	glm::vec3 coverage_max = { 10.0f, 10.0f, 10.0f };
	glm::ivec3 divisions = { 10, 5, 10 };

	std::vector<glm::vec4> m_probe_data;
	std::unique_ptr<GLBuffer> m_probe_buf;
	void allocate_probes();

	GLDynBuffer m_constant;
	void updateConstant();
	
	ReflectionMap reflection;
};