#pragma once

#include <vector>
#include <memory>
#include "core/Object3D.h"

class Mesh;
class GLTFModel : public Object3D
{
public:
	std::vector<Mesh> m_meshs;
};