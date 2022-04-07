#include <GL/glew.h>
#include "TestRoutine.h"
#include "materials/MeshStandardMaterial.h"

static std::string s_vertex_common =
R"(#version 430
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
};

layout (std140, binding = 1) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (location = 0) out vec3 vPos;
layout (location = 1) out vec3 vNorm;

#GLOBAL#

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	gl_Position = uProjMat*(uViewMat*wolrd_pos);
	vPos = wolrd_pos.xyz;
	vec4 world_norm = uNormalMat * vec4(aNorm, 0.0);
	vNorm =  normalize(world_norm.xyz);

#MAIN#
}
)";

static std::string s_vertex_color[2] =
{
R"(
layout (location = 2) in vec3 aColor;
layout (location = 2) out vec3 vColor;
)",
R"(	vColor = aColor;
)"
};

static std::string s_vertex_uv[2] =
{
R"(
layout (location = 3) in vec2 aUV;
layout (location = 3) out vec2 vUV;
)",
R"(	vUV = aUV;
)"
};

static std::string s_frag_common =
R"(#version 430
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (std140, binding = 2) uniform Material
{
	vec3 uColor;
};

#GLOBAL#

out vec4 outColor;void main()
{
	vec3 norm = normalize(vNorm);
	vec3 light_dir = normalize(vec3(1.0f, 2.0f, 1.0f));
	float l_diffuse = clamp(dot(light_dir, norm), 0.0, 1.0);
	float l_ambient = 0.3;
	vec3 col = uColor;
#MAIN#
	col *= (l_diffuse + l_ambient);
	outColor = vec4(col, 1.0);
}
)";

static std::string s_frag_color[2] =
{
R"(
layout (location = 2) in vec3 vColor;
)",
R"(	col *= vColor;
)"
};

static std::string s_frag_color_tex[2] =
{
R"(
layout (location = 3) in vec2 vUV;
layout (location = 0) uniform sampler2D uTexColor;
)",
R"(	col *= texture(uTexColor, vUV).xyz;
)"
};

inline void replace(std::string& str, const char* target, const char* src)
{
	size_t pos = str.find(target);
	str.replace(pos, strlen(target), src);
}

void TestRoutine::s_generate_shaders(const Options& options, std::string& s_vertex, std::string& s_frag)
{
	s_vertex = s_vertex_common;
	s_frag = s_frag_common;

	std::string s_v_global = "";
	std::string s_v_main = "";
	std::string s_f_global = "";
	std::string s_f_main = "";

	if (options.has_color)
	{
		s_v_global += s_vertex_color[0];
		s_v_main += s_vertex_color[1];
		s_f_global += s_frag_color[0];
		s_f_main += s_frag_color[1];
	}

	if (options.has_color_texture)
	{
		s_v_global += s_vertex_uv[0];
		s_v_main += s_vertex_uv[1];
		s_f_global += s_frag_color_tex[0];
		s_f_main += s_frag_color_tex[1];
	}

	replace(s_vertex, "#GLOBAL#", s_v_global.c_str());
	replace(s_vertex, "#MAIN#", s_v_main.c_str());
	replace(s_frag, "#GLOBAL#", s_f_global.c_str());
	replace(s_frag, "#MAIN#", s_f_main.c_str());
}

TestRoutine::TestRoutine(const Options& options) : m_options(options)
{
	std::string s_vertex, s_frag;
	s_generate_shaders(options, s_vertex, s_frag);
	
	m_vert_shader = std::unique_ptr<GLShader>(new GLShader(GL_VERTEX_SHADER, s_vertex.c_str()));
	m_frag_shader = std::unique_ptr<GLShader>(new GLShader(GL_FRAGMENT_SHADER, s_frag.c_str()));
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(*m_vert_shader, *m_frag_shader));
}

void TestRoutine::render(const RenderParams& params)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_model->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, material.constant_material.m_id);

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, geo.normal_buf->m_id);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	if (m_options.has_color)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->color_buf->m_id);
		glVertexAttribPointer(2, params.primitive->type_color, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);
	}

	if (m_options.has_color_texture)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->uv_buf->m_id);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(3);
	
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_map];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(0, 0);
	}	

	if (params.primitive->index_buf != nullptr)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, params.primitive->index_buf->m_id);
		if (params.primitive->type_indices == 2)
		{
			glDrawElements(GL_TRIANGLES, params.primitive->num_face * 3, GL_UNSIGNED_SHORT, nullptr);
		}
		else if (params.primitive->type_indices == 4)
		{
			glDrawElements(GL_TRIANGLES, params.primitive->num_face * 3, GL_UNSIGNED_INT, nullptr);
		}
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, params.primitive->num_pos);
	}

	glUseProgram(0);
}

