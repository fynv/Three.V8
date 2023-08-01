#pragma once

#include <memory>
#include <string>
#include <vector>

#include "materials/MeshStandardMaterial.h"
#include "renderers/GLUtils.h"

class Primitive;
class Camera;
class NormalAndDepth
{
public:
	NormalAndDepth(bool has_normal_map);

	struct RenderParams
	{
		const GLTexture2D** tex_list;
		const MeshStandardMaterial** material_list;
		const Camera* camera;
		const GLBuffer* constant_model;
		const Primitive* primitive;
	};

	void render(const RenderParams& params);
	void render_batched(const RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst);

private:
	bool m_has_normal_map;
	std::unique_ptr<GLProgram> m_prog;

};

