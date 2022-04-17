#pragma once

#include <string>
#include <glm.hpp>

#include "materials/MeshStandardMaterial.h"
class GLTexture2D;
class GLDynBuffer;
class Primitive;
class ConstDirectionalLight;

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

	struct Lights
	{
		int num_directional_lights;
		const ConstDirectionalLight* directional_lights;
	};

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

	std::unique_ptr<GLDynBuffer> m_constant_directional_lights;
};
