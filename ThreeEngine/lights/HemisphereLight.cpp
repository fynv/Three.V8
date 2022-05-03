#include <GL/glew.h>
#include "HemisphereLight.h"

struct HemisphereLightConst
{
	glm::vec4 hemisphereSkyColor;
	glm::vec4 hemisphereGroundColor;
	float diffuseThresh;
	float diffuseHigh;
	float diffuseLow;
	float specularThresh;
	float specularHigh;
	float specularLow;
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
	c.diffuseThresh = diffuse_thresh;
	c.diffuseHigh = diffuse_high;
	c.diffuseLow = diffuse_low;
	c.specularThresh = specular_thresh;
	c.specularHigh = specular_high;
	c.specularLow = specular_low;
	m_constant.upload(&c);
}
