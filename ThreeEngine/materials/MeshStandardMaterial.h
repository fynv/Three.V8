#pragma once

#include "Material.h"
#include <glm.hpp>
#include "renderers/GLUtils.h"

class MeshStandardMaterial : public Material
{
public:
	MeshStandardMaterial();
	glm::vec3 color;
	glm::vec2 normalScale;
	int tex_idx_map = -1;
	int tex_idx_metalnessMap = -1; // blue channel
	int tex_idx_roughnessMap = -1; // green channel 
	int tex_idx_normalMap = -1;

	float metallicFactor = 0.0f;
	float roughnessFactor = 1.0f;

	GLBuffer constant_material;
	void update_uniform();
};