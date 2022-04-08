#pragma once

#include <string>
#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "materials/MeshStandardMaterial.h"
#include "models/ModelComponents.h"

class StandardRoutine
{
public:	
	struct Options
	{
		bool has_color;
		bool has_color_texture;		
	};

	StandardRoutine(const Options& options);

	struct RenderParams
	{
		const GLTexture2D** tex_list;
		const MeshStandardMaterial** material_list;
		const GLDynBuffer* constant_camera;
		const GLDynBuffer* constant_model;
		const Primitive* primitive;
	};

	void render(const RenderParams& params);

private:
	Options m_options;
	static void s_generate_shaders(const Options& options, std::string& s_vertex, std::string& s_frag);

	std::unique_ptr<GLShader> m_vert_shader;
	std::unique_ptr<GLShader> m_frag_shader;
	std::unique_ptr<GLProgram> m_prog;

	
};
