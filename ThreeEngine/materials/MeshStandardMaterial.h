#pragma once

#include "Material.h"
#include <glm.hpp>
#include "renderers/GLUtils.h"

class MeshStandardMaterial : public Material
{
public:
	MeshStandardMaterial();
	glm::vec3 color;
	int tex_idx_map = 0;
	GLBuffer constant_material;
	void update_uniform();
};