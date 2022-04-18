#pragma once

#include <string>
#include <glm.hpp>

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
		int num_directional_lights = 0;
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
	static void s_generate_shaders(const Options& options, std::string& s_vertex, std::string& s_frag);

	std::unique_ptr<GLShader> m_vert_shader;
	std::unique_ptr<GLShader> m_frag_shader;
	std::unique_ptr<GLProgram> m_prog;
};
