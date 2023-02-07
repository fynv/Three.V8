#include <GL/glew.h>
#include "EnvironmentMap.h"

struct EnvironmentMapConst
{
	glm::vec4 shCoefficients[9];
	float diffuseThresh;
	float diffuseHigh;
	float diffuseLow;
	float specularThresh;
	float specularHigh;
	float specularLow;
};

EnvironmentMap::EnvironmentMap(): m_constant(sizeof(EnvironmentMapConst), GL_UNIFORM_BUFFER)
{
	memset(shCoefficients, 0, sizeof(glm::vec4) * 9);
}


EnvironmentMap::~EnvironmentMap()
{
	
}

void EnvironmentMap::updateConstant()
{
	EnvironmentMapConst c;
	memcpy(c.shCoefficients, shCoefficients, sizeof(glm::vec4) * 9);
	c.diffuseThresh = diffuse_thresh;
	c.diffuseHigh = diffuse_high;
	c.diffuseLow = diffuse_low;
	c.specularThresh = specular_thresh;
	c.specularHigh = specular_high;
	c.specularLow = specular_low;
	m_constant.upload(&c);
}