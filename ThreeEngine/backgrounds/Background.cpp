#include <GL/glew.h>
#include "Background.h"

struct HemisphereBackgroundConst
{
	glm::vec4 hemisphereSkyColor;
	glm::vec4 hemisphereGroundColor;
};

HemisphereBackground::HemisphereBackground()
	: m_constant(sizeof(HemisphereBackgroundConst), GL_UNIFORM_BUFFER)
{

}

void HemisphereBackground::updateConstant()
{
	HemisphereBackgroundConst c;
	c.hemisphereSkyColor = glm::vec4(skyColor, 1.0f);
	c.hemisphereGroundColor = glm::vec4(groundColor, 1.0f);
	m_constant.upload(&c);
}
