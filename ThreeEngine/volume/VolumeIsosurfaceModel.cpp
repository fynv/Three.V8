#include <GL/glew.h>
#include "VolumeIsosurfaceModel.h"

struct ModelConst
{
	glm::mat4 invModelMat;
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
	glm::ivec4 size;
	glm::vec4 spacing;
	glm::vec4 color;
	float metallicFactor;
	float roughnessFactor;	
	float step;
	float isovalue;
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
	c.spacing = glm::vec4(m_data->spacing, 0.0);
	c.color = m_material.color;
	c.metallicFactor = m_material.metallicFactor;
	c.roughnessFactor = m_material.roughnessFactor;	
	c.step = glm::length(m_data->spacing) / 2.0f;
	c.isovalue = m_isovalue;
	m_constant.upload(&c);
}


void VolumeIsosurfaceModel::set_color(const glm::vec3& color)
{
	m_material.color = glm::vec4(color, 1.0f);
	updateConstant();

}


void VolumeIsosurfaceModel::set_metalness(float metalness)
{
	m_material.metallicFactor = metalness;
	updateConstant();
}


void VolumeIsosurfaceModel::set_roughness(float roughness)
{
	m_material.roughnessFactor = roughness;
	updateConstant();
}
