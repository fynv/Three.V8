#pragma once

#include "core/Object3D.h"
#include "models/ModelComponents.h"
#include "materials/MeshStandardMaterial.h"
#include "renderers/GLUtils.h"

class MeshStandardMaterial;
class SimpleModel : public Object3D
{
public:
	SimpleModel();
	
	GLDynBuffer m_constant;
	void updateConstant();

	GLTexture2D texture;
	MeshStandardMaterial material;
	Primitive geometry;

	void set_color(const glm::vec3& color);
	void set_metalness(float metalness);
	void set_roughness(float roughness);

	void set_toon_shading(int mode, float wire_width);
};


