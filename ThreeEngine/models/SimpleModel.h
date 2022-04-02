#pragma once

#include <memory>
#include "core/Object3D.h"
#include "geometries/Geometry1.h"
#include "materials/MeshStandardMaterial.h"
#include "renderers/GLUtils.h"

class MeshStandardMaterial;
class SimpleModel : public Object3D
{
public:
	SimpleModel();
	GLTexture2D texture;
	MeshStandardMaterial material;
	Geometry1 geometry;

	void set_color(const glm::vec3& color);
};


