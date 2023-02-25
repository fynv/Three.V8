#pragma once

#include <vector>
#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "lights/IndirectLight.h"

class GLRenderer;
class Scene;
class LODProbeGrid : public IndirectLight
{
public:
	LODProbeGrid();
	~LODProbeGrid();

	glm::vec3 coverage_min = { -10.0f, 0.0f, -10.0f };
	glm::vec3 coverage_max = { 10.0f, 10.0f, 10.0f };
	glm::ivec3 base_divisions = { 10, 5, 10 };
	int sub_division_level = 2;

	std::vector<int> m_sub_index;
	std::unique_ptr<GLBuffer> m_sub_index_buf;

	std::vector<glm::vec4> m_probe_data;	
	std::unique_ptr<GLBuffer> m_probe_buf;

	int getNumberOfProbes();

	GLDynBuffer m_constant;
	void updateConstant();
	
	void initialize(GLRenderer& renderer, Scene& scene);

};