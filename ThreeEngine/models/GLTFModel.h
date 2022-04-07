#pragma once

#include <vector>
#include "models/Model.h"

class Mesh;
class GLTFModel : public Model
{
public:
	std::vector<Mesh> m_meshs;
};