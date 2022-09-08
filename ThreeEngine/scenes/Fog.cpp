#include <GL/glew.h>
#include "Fog.h"

struct FogConst
{
	glm::vec4 rgba;
	int max_num_steps;
	float min_step;
	int padding[2];
};

Fog::Fog()
	: m_constant(sizeof(FogConst), GL_UNIFORM_BUFFER)
{

}


void Fog::updateConstant()
{
	FogConst c;
	c.rgba = glm::vec4(color, density);
	c.max_num_steps = max_num_steps;
	c.min_step = min_step;
	m_constant.upload(&c);
}
