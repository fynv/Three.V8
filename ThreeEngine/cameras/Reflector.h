#pragma once

#include "core/Object3D.h"
#include "cameras/PerspectiveCamera.h"
#include "renderers/GLRenderTarget.h"
#include "renderers/GLUtils.h"
#include "renderers/routines/DepthDownsample.h"

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
	std::unique_ptr<GLTexture2D> m_tex_depth_1x;
	unsigned m_fbo_depth_1x = 0;
	DepthDownsample m_depth_downsampler;
	PerspectiveCamera m_camera;

	void updateTarget(int width, int height);
	void depthDownsample();

	void calc_scissor();
};



