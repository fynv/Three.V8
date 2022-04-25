#pragma once

#include <string>

#include "materials/MeshStandardMaterial.h"
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class Primitive;
class StandardRoutine
{
public:	
	struct Options
	{
		AlphaMode alpha_mode = AlphaMode::Opaque;
		bool is_highlight_pass = false;
		bool has_color = false;
		bool has_color_texture = false;		
		bool has_metalness_map = false;
		bool has_roughness_map = false;
		bool has_normal_map = false;
		bool has_emissive_map = false;
		int num_directional_lights = 0;
		int num_directional_shadows = 0;
		bool has_environment_map = false;
		bool has_ambient_light = false;
		bool has_hemisphere_light = false;
	};

	StandardRoutine(const Options& options);

	struct RenderParams
	{
		const GLTexture2D** tex_list;
		const MeshStandardMaterial** material_list;
		const GLDynBuffer* constant_camera;
		const GLDynBuffer* constant_model;
		const Primitive* primitive;
		const Lights* lights;
	};

	void render(const RenderParams& params);

private:
	Options m_options;

	struct Bindings
	{
		int location_attrib_pos;
		int location_attrib_norm;
		int binding_camera;
		int binding_model;
		int location_varying_viewdir;
		int location_varying_norm;
		int binding_material;
		int location_attrib_color;
		int location_varying_color;
		int location_attrib_uv;
		int location_varying_uv;
		int location_tex_color;
		int location_tex_metalness;
		int location_tex_roughness;
		int location_tex_normal;
		int location_attrib_tangent;
		int location_varying_tangent;
		int location_attrib_bitangent;
		int location_varying_bitangent;
		int location_tex_emissive;
		int location_varying_world_pos;
		int binding_directional_lights;
		int location_tex_directional_shadow;
		int binding_environment_map;
		int location_tex_reflection_map;
		int binding_ambient_light;
		int binding_hemisphere_light;
	};

	Bindings m_bindings;

	static void s_generate_shaders(const Options& options, Bindings& bindings, std::string& s_vertex, std::string& s_frag);

	std::unique_ptr<GLShader> m_vert_shader;
	std::unique_ptr<GLShader> m_frag_shader;
	std::unique_ptr<GLProgram> m_prog;
};
