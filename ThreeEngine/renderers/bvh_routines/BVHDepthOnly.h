#pragma once

#include <memory>
#include <string>

#include "materials/MeshStandardMaterial.h"
#include "renderers/GLUtils.h"

class Primitive;
class BVHRenderTarget;
class BVHDepthOnly
{
public:
	BVHDepthOnly();

	struct RenderParams
	{
		const MeshStandardMaterial** material_list;
		const Primitive* primitive;
		const BVHRenderTarget* target;
		const GLDynBuffer* constant_camera;
	};

	void render(const RenderParams& params);

private:
	std::unique_ptr<GLProgram> m_prog;

};

