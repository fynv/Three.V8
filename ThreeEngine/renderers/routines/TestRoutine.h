#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"
#include "materials/Material.h"
#include "models/ModelComponents.h"

class TestRoutine
{
public:	
	TestRoutine();
	void render(const GLTexture2D** tex_list, const Material** material_list, const GLDynBuffer* constant_camera, const GLDynBuffer* constant_model, const Primitive& primitive);
	

private:
	static const char* s_vertex_shader;
	static const char* s_frag_shader;
	std::unique_ptr<GLProgram> m_prog;
	GLShader m_vert_shader;
	GLShader m_frag_shader;
};

class TestRoutine2
{
public:
	TestRoutine2();
	void render(const GLTexture2D** tex_list, const Material** material_list, const GLDynBuffer* constant_camera, const GLDynBuffer* constant_model, const Primitive& primitive);

private:
	static const char* s_vertex_shader;
	static const char* s_frag_shader;
	std::unique_ptr<GLProgram> m_prog;
	GLShader m_vert_shader;
	GLShader m_frag_shader;
};