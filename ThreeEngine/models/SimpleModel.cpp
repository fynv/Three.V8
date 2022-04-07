#include <GL/glew.h>
#include "SimpleModel.h"
#include "materials/MeshStandardMaterial.h"


struct ModelConst
{
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
};

SimpleModel::SimpleModel()
{
	glm::u8vec3 white = { 255, 255, 255 };
	texture.load_memory_rgb(1, 1, (uint8_t*)&white, true);
	material.tex_idx_map = 0;
	set_color({ 0.8f, 0.8f, 0.8f });
	geometry.material_idx = 0;
}

void SimpleModel::set_color(const glm::vec3& color)
{
	material.color = color;
	material.update_uniform();
}
