#pragma once

#include "models/Model.h"
#include "models/ModelComponents.h"
#include "materials/MeshStandardMaterial.h"
#include "renderers/GLUtils.h"

class MeshStandardMaterial;
class SimpleModel : public Model
{
public:
	SimpleModel();
	GLTexture2D texture;
	MeshStandardMaterial material;
	Primitive geometry;

	void set_color(const glm::vec3& color);
	void set_metalness(float metalness);
	void set_roughness(float roughness);
};


