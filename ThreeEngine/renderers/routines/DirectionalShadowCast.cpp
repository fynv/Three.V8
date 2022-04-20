#include <GL/glew.h>
#include "models/ModelComponents.h"
#include "lights/DirectionalLightShadow.h"
#include "DirectionalShadowCast.h"

static std::string g_vertex =
R"(#version 430
layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform Shadow
{
	mat4 uProjMat;
	mat4 uViewMat;	
};

layout (std140, binding = 1) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

#DEFINES#

#if ALPHA
#if HAS_COLOR
layout (location = 1) in vec4 aColor;
layout (location = 0) out float vAlpha;
#endif

#if HAS_UV
layout (location = 2) in vec2 aUV;
layout (location = 1) out vec2 vUV;
#endif
#endif

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	gl_Position = uProjMat*(uViewMat*wolrd_pos);	
	
#if ALPHA
#if HAS_COLOR
	vAlpha = aColor.w;
#endif

#if HAS_UV
	vUV = aUV;
#endif
#endif
}
)";

static std::string g_frag =
R"(#version 430

#DEFINES#

#if ALPHA
layout (std140, binding = 2) uniform Material
{
	vec4 uColor;
	vec4 uEmissive;
	vec2 uNormalScale;
	float uMetallicFactor;
	float uRoughnessFactor;
	float uAlphaCutoff;
};


#if HAS_COLOR
layout (location = 0) in float vAlpha;
#endif

#if HAS_UV
layout (location = 1) in vec2 vUV;
#endif

#if HAS_COLOR_TEX
layout (location = 0) uniform sampler2D uTexColor;
#endif

#endif

void main()
{
#if ALPHA
	float base_alpha = uColor.w;
#if HAS_COLOR
	base_alpha *= vAlpha;
#endif

#if HAS_COLOR_TEX
	base_alpha *= texture(uTexColor, vUV).w;
#endif

#if ALPHA_MASK
	base_alpha = base_alpha > uAlphaCutoff ? 1.0 : 0.0;
#endif

	if (base_alpha<0.5) discard;
#endif
}
)";

inline void replace(std::string& str, const char* target, const char* source)
{
	int start = 0;
	size_t target_len = strlen(target);
	size_t source_len = strlen(source);
	while (true)
	{
		size_t pos = str.find(target, start);
		if (pos == std::string::npos) break;
		str.replace(pos, target_len, source);
		start = pos + source_len;
	}
}


void DirectionalShadowCast::s_generate_shaders(const Options& options, std::string& s_vertex, std::string& s_frag)
{
	s_vertex = g_vertex;
	s_frag = g_frag;

	std::string defines = "";
	
	if (options.alpha_mode == AlphaMode::Opaque)
	{
		defines += "#define ALPHA 0\n";
	}


	if (options.alpha_mode == AlphaMode::Mask)
	{
		defines += "#define ALPHA 1\n";
		defines += "#define ALPHA_MASK 1\n";
	}
	else
	{
		defines += "#define ALPHA_MASK 0\n";
	}

	if (options.alpha_mode == AlphaMode::Blend)
	{
		defines += "#define ALPHA 1\n";
	}	

	if (options.has_color)
	{
		defines += "#define HAS_COLOR 1\n";
	}
	else
	{
		defines += "#define HAS_COLOR 0\n";
	}	

	if (options.has_color_texture)
	{
		defines += "#define HAS_UV 1\n";
		defines += "#define HAS_COLOR_TEX 1\n";
	}
	else
	{
		defines += "#define HAS_UV 0\n";
		defines += "#define HAS_COLOR_TEX 0\n";
	}	

	replace(s_vertex, "#DEFINES#", defines.c_str());
	replace(s_frag, "#DEFINES#", defines.c_str());
}


DirectionalShadowCast::DirectionalShadowCast(const Options& options) : m_options(options)
{
	std::string s_vertex, s_frag;
	s_generate_shaders(options, s_vertex, s_frag);

	m_vert_shader = std::unique_ptr<GLShader>(new GLShader(GL_VERTEX_SHADER, s_vertex.c_str()));
	m_frag_shader = std::unique_ptr<GLShader>(new GLShader(GL_FRAGMENT_SHADER, s_frag.c_str()));
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(*m_vert_shader, *m_frag_shader));
}


void DirectionalShadowCast::render(const RenderParams& params)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	glEnable(GL_CULL_FACE);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_shadow->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_model->m_id);

	if (m_options.alpha_mode != AlphaMode::Opaque)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, material.constant_material.m_id);
	}

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	if (m_options.alpha_mode != AlphaMode::Opaque)
	{
		if (m_options.has_color)
		{
			glBindBuffer(GL_ARRAY_BUFFER, params.primitive->color_buf->m_id);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(1);
		}

		if (m_options.has_color_texture)
		{
			glBindBuffer(GL_ARRAY_BUFFER, params.primitive->uv_buf->m_id);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(2);
		}

		if (m_options.has_color_texture)
		{
			const GLTexture2D& tex = *params.tex_list[material.tex_idx_map];
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex.tex_id);
			glUniform1i(0, 0);
		}
	}	

	if (params.primitive->index_buf != nullptr)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, params.primitive->index_buf->m_id);
		if (params.primitive->type_indices == 1)
		{
			glDrawElements(GL_TRIANGLES, params.primitive->num_face * 3, GL_UNSIGNED_BYTE, nullptr);
		}
		else if (params.primitive->type_indices == 2)
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


