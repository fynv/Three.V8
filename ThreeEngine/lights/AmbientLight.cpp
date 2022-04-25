#include <GL/glew.h>
#include "AmbientLight.h"

struct AmbientLightConst
{
	glm::vec4 ambientColorIntensity;
};

AmbientLight::AmbientLight()
	: m_constant(sizeof(AmbientLightConst), GL_UNIFORM_BUFFER)
{

}

void AmbientLight::updateConstant()
{
	AmbientLightConst c;
	c.ambientColorIntensity = glm::vec4(color, intensity);
	m_constant.upload(&c);
}