#pragma once

#include "core/Object3D.h"
#include "cameras/PerspectiveCamera.h"
#include "renderers/GLRenderTarget.h"
#include "renderers/GLUtils.h"


class Reflector : public Object3D
{
public:
	Reflector();
	~Reflector();

	float width = 1.0;
	float height = 1.0;

	GLBuffer m_constant;
	void updateConstant();

	GLRenderTarget m_target;
	PerspectiveCamera m_camera;
};



