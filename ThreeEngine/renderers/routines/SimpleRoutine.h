#pragma once

#include <string>

#include "materials/MeshStandardMaterial.h"
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class Primitive;
class SimpleRoutine
{
public:
	struct Options
	{
		AlphaMode alpha_mode = AlphaMode::Opaque;
		bool is_highlight_pass = false;
		bool specular_glossiness = false;
		bool has_color = false;
		bool has_color_texture = false;
		bool has_metalness_map = false;
		bool has_roughness_map = false;		
		bool has_emissive_map = false;
		bool has_specular_map = false;
		bool has_glossiness_map = false;
		int num_directional_lights = 0;
		int num_directional_shadows = 0;
		bool has_environment_map = false;
		bool has_probe_grid = false;
		bool probe_reference_recorded = false;
		bool has_lod_probe_grid = false;
		bool has_ambient_light = false;
		bool has_hemisphere_light = false;		
		bool has_fog = false;
	};

	SimpleRoutine(const Options& options);

	struct RenderParams
	{
		const GLTexture2D** tex_list;
		const MeshStandardMaterial** material_list;
		const GLDynBuffer* constant_camera;
		const GLDynBuffer* constant_model;
		const Primitive* primitive;
		const Lights* lights;
		const GLDynBuffer* constant_fog;
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
		int location_tex_emissive;
		int location_tex_specular;
		int location_tex_glossiness;
		int location_varying_world_pos;
		int binding_directional_lights;
		int binding_directional_shadows;
		int location_tex_directional_shadow;		
		int binding_environment_map;
		int binding_probe_grid;
		int binding_probes;
		int location_tex_irradiance;
		int location_tex_visibility;
		int binding_probe_references;
		int binding_lod_probe_grid;
		int binding_lod_probes;
		int binding_lod_probe_indices;
		int binding_ambient_light;
		int binding_hemisphere_light;
		int binding_fog;
	};

	Bindings m_bindings;

	static void s_generate_shaders(const Options& options, Bindings& bindings, std::string& s_vertex, std::string& s_frag);

	std::unique_ptr<GLProgram> m_prog;
};
