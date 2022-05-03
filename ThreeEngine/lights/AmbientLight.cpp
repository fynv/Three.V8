#include <GL/glew.h>
#include "AmbientLight.h"

struct AmbientLightConst
{
	glm::vec4 ambientColor;
	float diffuseThresh;
	float diffuseHigh;
	float diffuseLow;
	float specularThresh;
	float specularHigh;
	float specularLow;
};

AmbientLight::AmbientLight()
	: m_constant(sizeof(AmbientLightConst), GL_UNIFORM_BUFFER)
{

}

void AmbientLight::updateConstant()
{
	AmbientLightConst c;
	c.ambientColor = glm::vec4(color* intensity, 1.0f);
	c.diffuseThresh = diffuse_thresh;
	c.diffuseHigh = diffuse_high;
	c.diffuseLow = diffuse_low;
	c.specularThresh = specular_thresh;
	c.specularHigh = specular_high;
	c.specularLow = specular_low;
	m_constant.upload(&c);
}