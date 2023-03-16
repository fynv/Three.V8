#pragma once

#include <memory>
#include <string>

#include "materials/MeshStandardMaterial.h"
#include "renderers/GLUtils.h"

class Primitive;
class BVHRenderTarget;
class ProbeRayList;

class BVHDepthOnly
{
public:
	BVHDepthOnly(bool to_probe = false);

	struct RenderParams
	{
		const MeshStandardMaterial** material_list;
		const Primitive* primitive;
		const BVHRenderTarget* target;
		const GLDynBuffer* constant_camera;
		const ProbeRayList* prl;
	};

	void render(const RenderParams& params);

private:
	bool m_to_probe;	
	std::unique_ptr<GLProgram> m_prog;

};

