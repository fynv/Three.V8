#include <GL/glew.h>
#include "EnvironmentMap.h"

struct EnvironmentMapConst
{
	glm::vec4 shCoefficients[9];
};

EnvironmentMap::EnvironmentMap()
	: m_constant(sizeof(EnvironmentMapConst), GL_UNIFORM_BUFFER)
{


}


void EnvironmentMap::updateConstant()
{
	EnvironmentMapConst c;
	memcpy(c.shCoefficients, shCoefficients, sizeof(glm::vec4) * 9);
	m_constant.upload(&c);
}