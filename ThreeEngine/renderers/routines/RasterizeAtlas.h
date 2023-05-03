#pragma once

#include <string>

#include "materials/MeshStandardMaterial.h"
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class Primitive;
class RasterizeAtlas
{
public:
	RasterizeAtlas(bool has_normal_map);

	struct RenderParams
	{
		const GLTexture2D** tex_list;
		const MeshStandardMaterial** material_list;
		const GLBuffer* constant_model;
		const Primitive* primitive;
	};

	void render(const RenderParams& params);

private:
	bool m_has_normal_map;

	struct Bindings
	{
		int location_attrib_pos;
		int location_attrib_norm;
		int binding_model;
		int location_varying_world_pos;
		int location_varying_norm;
		int binding_material;
		int location_attrib_uv;
		int location_varying_uv;
		int location_attrib_atlas_uv;
		int location_tex_normal;
		int location_attrib_tangent;
		int location_varying_tangent;
		int location_attrib_bitangent;
		int location_varying_bitangent;
	};

	Bindings m_bindings;

	static void s_generate_shaders(bool has_normal_map, Bindings& bindings, std::string& s_vertex, std::string& s_frag);

	std::unique_ptr<GLProgram> m_prog;
};
