#pragma once

#include "core/Object3D.h"
#include "models/ModelComponents.h"
#include "materials/MeshStandardMaterial.h"
#include "renderers/GLUtils.h"

class Reflector;
class MeshStandardMaterial;
class SimpleModel : public Object3D
{
public:
	SimpleModel();
	
	GLBuffer m_constant;
	void updateConstant();

	GLTexture2D texture;
	GLTexture2D* repl_texture = nullptr;
	MeshStandardMaterial material;
	Primitive geometry;

	void set_color(const glm::vec3& color);
	void set_metalness(float metalness);
	void set_roughness(float roughness);

	void set_toon_shading(int mode, float wire_width, const glm::vec3& wire_color);

	Reflector* reflector = nullptr;
};


