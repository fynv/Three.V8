#pragma once

#include "core/Object3D.h"
#include "renderers/GLUtils.h"

class Model : public Object3D
{
public:
	Model();
	GLDynBuffer m_constant;
	void updateConstant();
};


