#pragma once

#include <memory>
#include <string>

#include "materials/MeshStandardMaterial.h"
#include "renderers/GLUtils.h"

class Primitive;
class DepthOnly
{
public:
	DepthOnly();

	struct RenderParams
	{		
		const MeshStandardMaterial** material_list;
		const GLDynBuffer* constant_camera;
		const GLDynBuffer* constant_model;
		const Primitive* primitive;
	};

	void render(const RenderParams& params);
	void render_batched(const RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst);

private:	
	std::unique_ptr<GLProgram> m_prog;

};

