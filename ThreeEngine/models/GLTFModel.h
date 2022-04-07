#pragma once

#include <memory>
#include <vector>
#include "models/Model.h"
#include "models/ModelComponents.h"
#include "renderers/GLUtils.h"
#include "materials/MeshStandardMaterial.h"

class Mesh;
class GLTFModel : public Model
{
public:
	std::vector<std::unique_ptr<GLTexture2D>> m_textures;
	std::vector<std::unique_ptr<MeshStandardMaterial>> m_materials;
	std::vector<Mesh> m_meshs;
};
