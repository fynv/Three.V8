#pragma once

#include <vector>
#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "lights/IndirectLight.h"

class Scene;
class ProbeGrid : public IndirectLight
{
public:
	ProbeGrid();
	~ProbeGrid();

	glm::vec3 coverage_min = { -10.0f, 0.0f, -10.0f };
	glm::vec3 coverage_max = { 10.0f, 10.0f, 10.0f };
	glm::ivec3 divisions = { 10, 5, 10 };
	float ypower = 1.0f;
	int vis_res = 16;
	int pack_size = 0;
	int pack_res = 0;

	std::vector<glm::vec4> m_probe_data;
	std::unique_ptr<GLBuffer> m_probe_buf;
	std::vector<unsigned short> m_visibility_data;
	std::unique_ptr<GLTexture2D> m_tex_visibility;
	void allocate_probes();

	GLDynBuffer m_constant;
	void updateConstant();

	void construct_visibility(Scene& scene);

	bool record_references = false;
	std::unique_ptr<GLBuffer> m_ref_buf;
	void set_record_references(bool record);	
	void get_references(std::vector<unsigned>& references);
};

