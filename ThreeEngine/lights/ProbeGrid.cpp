#include <GL/glew.h>
#include "ProbeGrid.h"

struct ProbeGridConst
{
	glm::vec4 coverageMin;
	glm::vec4 coverageMax;
	glm::ivec4 divisions;	
	float diffuseThresh;
	float diffuseHigh;
	float diffuseLow;
	float specularThresh;
	float specularHigh;
	float specularLow;
};

ProbeGrid::ProbeGrid() : m_constant(sizeof(ProbeGridConst), GL_UNIFORM_BUFFER)
{
	
}


ProbeGrid::~ProbeGrid()
{

}

void ProbeGrid::allocate_probes()
{	
	size_t num = divisions.x * divisions.y * divisions.z;
	size_t size = sizeof(glm::vec4) * 9 * num;
	m_probe_buf = std::unique_ptr<GLBuffer>(new GLBuffer(size, GL_SHADER_STORAGE_BUFFER));
	m_probe_data.resize(9 * num, glm::vec4(0.0f));
	m_probe_buf->upload(m_probe_data.data());
}

void ProbeGrid::updateConstant()
{
	ProbeGridConst c;
	c.coverageMin = glm::vec4(coverage_min, 0.0f);
	c.coverageMax = glm::vec4(coverage_max, 0.0f);
	c.divisions = glm::ivec4(divisions, 0);
	c.diffuseThresh = diffuse_thresh;
	c.diffuseHigh = diffuse_high;
	c.diffuseLow = diffuse_low;
	c.specularThresh = specular_thresh;
	c.specularHigh = specular_high;
	c.specularLow = specular_low;
	m_constant.upload(&c);
}