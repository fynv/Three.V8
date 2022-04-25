#include <GL/glew.h>
#include "AmbientLight.h"

struct AmbientLightConst
{
	glm::vec4 ambientColor;
};

AmbientLight::AmbientLight()
	: m_constant(sizeof(AmbientLightConst), GL_UNIFORM_BUFFER)
{

}

void AmbientLight::updateConstant()
{
	AmbientLightConst c;
	c.ambientColor = glm::vec4(color* intensity, 1.0f);
	m_constant.upload(&c);
}