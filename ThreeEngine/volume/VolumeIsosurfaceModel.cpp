#include <GL/glew.h>
#include "VolumeIsosurfaceModel.h"

struct ModelConst
{
	glm::mat4 invModelMat;
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
	glm::ivec4 size;
	glm::vec3 spacing;
	float m_isovalue;
};

VolumeIsosurfaceModel::VolumeIsosurfaceModel(VolumeData* data)
	: m_data(data)
	, m_constant(sizeof(ModelConst), GL_UNIFORM_BUFFER)
{


}

VolumeIsosurfaceModel::~VolumeIsosurfaceModel()
{


}



void VolumeIsosurfaceModel::updateConstant()
{
	ModelConst c;
	c.invModelMat = glm::inverse(matrixWorld);
	c.ModelMat = matrixWorld;
	c.NormalMat = glm::transpose(c.invModelMat);
	c.size = glm::ivec4(m_data->size, 0);
	c.spacing = m_data->spacing;
	c.m_isovalue = m_isovalue;
	m_constant.upload(&c);
}