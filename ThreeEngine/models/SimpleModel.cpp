#include <GL/glew.h>
#include "SimpleModel.h"
#include "materials/MeshStandardMaterial.h"

struct ModelConst
{
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
};

SimpleModel::SimpleModel() : m_constant(sizeof(ModelConst), GL_UNIFORM_BUFFER)
{
	glm::u8vec3 white = { 255, 255, 255 };
	texture.load_memory_bgr(1, 1, (uint8_t*)&white, true);
	material.tex_idx_map = 0;
	set_color({ 0.8f, 0.8f, 0.8f });
	geometry.material_idx = 0;
}


void SimpleModel::updateConstant()
{
	ModelConst c;
	c.ModelMat = matrixWorld;
	c.NormalMat = glm::transpose(glm::inverse(matrixWorld));
	m_constant.upload(&c);
}

void SimpleModel::set_color(const glm::vec3& color)
{
	material.color = glm::vec4(color, 1.0f);
	material.update_uniform();
}


void SimpleModel::set_metalness(float metalness)
{
	material.metallicFactor = metalness;
	material.update_uniform();
}


void SimpleModel::set_roughness(float roughness)
{
	material.roughnessFactor = roughness;
	material.update_uniform();
}

void SimpleModel::set_toon_shading(int mode, float wire_width, const glm::vec3& wire_color)
{
	material.tone_shading = mode;
	material.wire_width = wire_width;
	material.wire_color = wire_color;
}