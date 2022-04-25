#include <GL/glew.h>
#include "HemisphereLight.h"

struct HemisphereLightConst
{
	glm::vec4 hemisphereSkyColor;
	glm::vec4 hemisphereGroundColor;
};

HemisphereLight::HemisphereLight()
	: m_constant(sizeof(HemisphereLightConst), GL_UNIFORM_BUFFER)
{

}

void HemisphereLight::updateConstant()
{
	HemisphereLightConst c;
	c.hemisphereSkyColor = glm::vec4(skyColor * intensity, 1.0f);
	c.hemisphereGroundColor = glm::vec4(groundColor * intensity, 1.0f);
	m_constant.upload(&c);
}
