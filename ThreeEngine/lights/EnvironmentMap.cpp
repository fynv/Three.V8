#include <GL/glew.h>
#include "EnvironmentMap.h"

struct EnvironmentMapConst
{
	glm::vec4 shCoefficients[9];
};

EnvironmentMap::EnvironmentMap()
	: m_constant(sizeof(EnvironmentMapConst), GL_UNIFORM_BUFFER)
{
	glGenTextures(1, &id_cube_reflection);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id_cube_reflection);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 7, GL_RGBA8, 128, 128);

}

EnvironmentMap::~EnvironmentMap()
{
	glDeleteTextures(1, &id_cube_reflection);
}

void EnvironmentMap::updateConstant()
{
	EnvironmentMapConst c;
	memcpy(c.shCoefficients, shCoefficients, sizeof(glm::vec4) * 9);
	m_constant.upload(&c);
}