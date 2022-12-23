#include <GL/glew.h>
#include "VolumeIsosurfaceModel.h"
#include "routines/InitiGridPartition.h"

struct ModelConst
{
	glm::mat4 invModelMat;
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
	glm::ivec4 size;
	glm::vec4 spacing;
	glm::ivec4 bsize;
	glm::ivec4 bnum;
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
	m_partition = std::unique_ptr<GridPartition>(new GridPartition);
	InitGridPartition init(data->bytes_per_pixel);
	init.Init(*data, *m_partition, 0.4f);

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
	c.bsize = glm::ivec4(m_partition->bsize, 0);
	c.bnum = glm::ivec4(m_partition->bnum, 0);
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

}


void VolumeIsosurfaceModel::set_metalness(float metalness)
{
	m_material.metallicFactor = metalness;
}


void VolumeIsosurfaceModel::set_roughness(float roughness)
{
	m_material.roughnessFactor = roughness;
}
