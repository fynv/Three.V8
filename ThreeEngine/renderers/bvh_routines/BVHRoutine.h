#pragma once

#include <memory>
#include <string>

#include "materials/MeshStandardMaterial.h"
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class Primitive;
class BVHRenderTarget;
class ProbeRayList;
class LightmapRayList;
class BVHRoutine
{
public:
	struct Options
	{
		Options()
		{
			memset(this, 0, sizeof(Options));
		}
		int target_mode = 0;
		AlphaMode alpha_mode = AlphaMode::Opaque;		
		bool has_lightmap = false;
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

	BVHRoutine(const Options& options);

	struct RenderParams
	{
		const GLTexture2D** tex_list;
		const MeshStandardMaterial** material_list;		
		const GLBuffer* constant_model;
		const Primitive* primitive;
		const Lights* lights;
		const GLTexture2D* tex_lightmap;
		const GLBuffer* constant_fog;

		const BVHRenderTarget* target;
		const GLBuffer* constant_camera;
		const ProbeRayList* prl;
		const LightmapRayList* lmrl;
	};

	void render(const RenderParams& params);

private:
	Options m_options;

	struct Bindings
	{
		int binding_material;
		int binding_model;
		int location_tex_faces;
		int binding_positions;
		int binding_normals;
		int binding_colors;
		int binding_uv;
		int binding_atlas_uv;
		int location_tex_color;
		int location_tex_metalness;
		int location_tex_roughness;
		int location_tex_emissive;
		int location_tex_specular;
		int location_tex_glossiness;
		int binding_directional_lights;
		int binding_directional_shadows;
		int location_tex_directional_shadow;
		int location_tex_lightmap;
		int binding_environment_map;
		int binding_probe_grid;		
		int location_tex_irradiance;
		int location_tex_visibility;
		int binding_probe_references;
		int binding_lod_probe_grid;
		int binding_lod_probes;
		int binding_lod_probe_indices;
		int binding_ambient_light;
		int binding_hemisphere_light;
		int binding_fog;
		int binding_camera;		
		int binding_prl;
		int binding_prl_pos;
		int binding_lightmap_ray_list;
		int location_tex_lightmap_pos;
		int location_tex_lightmap_norm;
		int location_tex_lightmap_valid_list;

	};

	Bindings m_bindings;

	static void s_generate_shaders(const Options& options, Bindings& bindings, std::string& s_compute);

	std::unique_ptr<GLProgram> m_prog;
};

