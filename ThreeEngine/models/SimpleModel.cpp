#include "SimpleModel.h"
#include "materials/MeshStandardMaterial.h"

SimpleModel::SimpleModel()
{
	set_color({ 0.8f, 0.8f, 0.8f });
	glm::u8vec3 white = { 255, 255, 255 };
	texture.load_memory_rgb(1, 1, (uint8_t*)&white, true);
	geometry.material_idx = 0;
}

void SimpleModel::set_color(const glm::vec3& color)
{
	material.color = color;
	material.update_uniform();
}


