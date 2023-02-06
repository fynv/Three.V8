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

EnvironmentMap::EnvironmentMap()
	: m_constant(sizeof(EnvironmentMapConst), GL_UNIFORM_BUFFER)
{
	glGenTextures(1, &id_cube_reflection);
}

void EnvironmentMap::allocate()
{
	if (!allocated)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, id_cube_reflection);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 7, GL_RGBA8, 128, 128);
		allocated = true;
	}
}

EnvironmentMap::~EnvironmentMap()
{
	glDeleteTextures(1, &id_cube_reflection);
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