#pragma once

#include <memory>
#include <string>

#include "materials/MeshStandardMaterial.h"
#include "renderers/GLUtils.h"

class Primitive;
class DirectionalShadowCast
{
public:
	struct Options
	{
		AlphaMode alpha_mode = AlphaMode::Opaque;
		bool has_color = false;
		bool has_color_texture = false;
	};

	DirectionalShadowCast(const Options& options);

	struct RenderParams
	{
		const GLTexture2D** tex_list;
		const MeshStandardMaterial** material_list;
		const GLDynBuffer* constant_shadow;
		const GLDynBuffer* constant_model;
		const Primitive* primitive;
	};

	void render(const RenderParams& params);


private:
	Options m_options;

	struct Bindings
	{
		int location_attrib_pos;
		int binding_shadow;
		int binding_model;
		int binding_material;
		int location_attrib_color;
		int location_varying_alpha;
		int location_attrib_uv;
		int location_varying_uv;
		int location_tex_color;		
	};
	Bindings m_bindings;

	static void s_generate_shaders(const Options& options, Bindings& bindings, std::string& s_vertex, std::string& s_frag);

	std::unique_ptr<GLProgram> m_prog;

};

