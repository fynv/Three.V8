#include <GL/glew.h>
#include "models/ModelComponents.h"
#include "DirectionalShadowCast.h"

static std::string g_vertex =
R"(#version 430
#DEFINES#

layout (location = LOCATION_ATTRIB_POS) in vec3 aPos;

layout (std140, binding = BINDING_SHADOW) uniform Shadow
{
	mat4 uVPSBMat;
	mat4 uProjMat;
	mat4 uViewMat;	
    vec2 uLeftRight;
	vec2 uBottomTop;
	vec2 uNearFar;
	float uLightRadius;
	float uBias;
};

layout (std140, binding = BINDING_MODEL) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (std140, binding = BINDING_MATERIAL) uniform Material
{
	vec4 uColor;
	vec4 uEmissive;
	vec4 uSpecularGlossiness;
	vec2 uNormalScale;
	float uMetallicFactor;
	float uRoughnessFactor;
	float uAlphaCutoff;
	int uDoubleSided;
};

#if ALPHA
#if HAS_COLOR
layout (location = LOCATION_ATTRIB_COLOR) in vec4 aColor;
layout (location = LOCATION_VARYING_ALPHA) out float vAlpha;
#endif

#if HAS_UV
layout (location = LOCATION_ATTRIB_UV) in vec2 aUV;
layout (location = LOCATION_VARYING_UV) out vec2 vUV;
#endif
#endif

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	gl_Position = uProjMat*(uViewMat*wolrd_pos);

	if (uDoubleSided!=0 && uBias > 0.0)
	{
		gl_Position.z += uBias * gl_Position.w;
	}
	
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
layout (std140, binding = BINDING_MATERIAL) uniform Material
{
	vec4 uColor;
	vec4 uEmissive;
	vec4 uSpecularGlossiness;
	vec2 uNormalScale;
	float uMetallicFactor;
	float uRoughnessFactor;
	float uAlphaCutoff;
	int uDoubleSided;
};


#if HAS_COLOR
layout (location = LOCATION_VARYING_ALPHA) in float vAlpha;
#endif

#if HAS_UV
layout (location = LOCATION_VARYING_UV) in vec2 vUV;
#endif

#if HAS_COLOR_TEX
layout (location = LOCATION_TEX_COLOR) uniform sampler2D uTexColor;
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


void DirectionalShadowCast::s_generate_shaders(const Options& options, Bindings& bindings, std::string& s_vertex, std::string& s_frag)
{
	s_vertex = g_vertex;
	s_frag = g_frag;

	std::string defines = "";

	{
		bindings.location_attrib_pos = 0;
		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_POS %d\n", bindings.location_attrib_pos);
			defines += line;
		}
	}

	{
		bindings.binding_shadow = 0;
		{
			char line[64];
			sprintf(line, "#define BINDING_SHADOW %d\n", bindings.binding_shadow);
			defines += line;
		}
	}

	{
		bindings.binding_model = bindings.binding_shadow + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_MODEL %d\n", bindings.binding_model);
			defines += line;
		}
	}	

	{
		bindings.binding_material = bindings.binding_model + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_MATERIAL %d\n", bindings.binding_material);
			defines += line;
		}
	}
	
	if (options.alpha_mode == AlphaMode::Opaque)
	{
		defines += "#define ALPHA 0\n";		
	}
	else
	{
		defines += "#define ALPHA 1\n";		

		if (options.alpha_mode == AlphaMode::Mask)
		{
			defines += "#define ALPHA_MASK 1\n";
		}
		else if (options.alpha_mode == AlphaMode::Blend)
		{
			defines += "#define ALPHA_MASK 0\n";
		}
	}

	if (options.has_color)
	{
		defines += "#define HAS_COLOR 1\n";

		bindings.location_attrib_color = bindings.location_attrib_pos + 1;
		bindings.location_varying_alpha = 0;
		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_COLOR %d\n", bindings.location_attrib_color);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_ALPHA %d\n", bindings.location_varying_alpha);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_COLOR 0\n";
		bindings.location_attrib_color = bindings.location_attrib_pos;
		bindings.location_varying_alpha = -1;
	}	

	if (options.has_color_texture)
	{
		defines += "#define HAS_UV 1\n";
		defines += "#define HAS_COLOR_TEX 1\n";

		bindings.location_attrib_uv = bindings.location_attrib_color + 1;
		bindings.location_varying_uv = bindings.location_varying_alpha + 1;
		bindings.location_tex_color = 0;

		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_UV %d\n", bindings.location_attrib_uv);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_UV %d\n", bindings.location_varying_uv);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_COLOR %d\n", bindings.location_tex_color);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_UV 0\n";
		defines += "#define HAS_COLOR_TEX 0\n";

		bindings.location_attrib_uv = bindings.location_attrib_color;
		bindings.location_varying_uv = bindings.location_varying_alpha;
		bindings.location_tex_color = -1;
	}	
	

	replace(s_vertex, "#DEFINES#", defines.c_str());
	replace(s_frag, "#DEFINES#", defines.c_str());
}


DirectionalShadowCast::DirectionalShadowCast(const Options& options) : m_options(options)
{
	std::string s_vertex, s_frag;
	s_generate_shaders(options, m_bindings, s_vertex, s_frag);

	GLShader vert_shader(GL_VERTEX_SHADER, s_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, s_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}

void DirectionalShadowCast::_render_common(const RenderParams& params)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_shadow, params.constant_shadow->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_model, params.constant_model->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_material, material.constant_material.m_id);

	if (m_options.alpha_mode != AlphaMode::Opaque)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_material, material.constant_material.m_id);
	}

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(m_bindings.location_attrib_pos, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(m_bindings.location_attrib_pos);

	if (m_options.alpha_mode != AlphaMode::Opaque)
	{
		if (m_options.has_color)
		{
			glBindBuffer(GL_ARRAY_BUFFER, params.primitive->color_buf->m_id);
			glVertexAttribPointer(m_bindings.location_attrib_color, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(m_bindings.location_attrib_color);
		}

		if (m_options.has_color_texture)
		{
			glBindBuffer(GL_ARRAY_BUFFER, params.primitive->uv_buf->m_id);
			glVertexAttribPointer(m_bindings.location_attrib_uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(m_bindings.location_attrib_uv);
		}

		if (m_options.has_color_texture)
		{
			const GLTexture2D& tex = *params.tex_list[material.tex_idx_map];
			glActiveTexture(GL_TEXTURE0 + m_bindings.location_tex_color);
			glBindTexture(GL_TEXTURE_2D, tex.tex_id);
			glUniform1i(m_bindings.location_tex_color, m_bindings.location_tex_color);
		}
	}
}

void DirectionalShadowCast::render(const RenderParams& params)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	if (material.doubleSided && !params.force_cull)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}

	glUseProgram(m_prog->m_id);
	
	_render_common(params);

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


void DirectionalShadowCast::render_batched(const RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst)
{

	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	if (material.doubleSided && !params.force_cull)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}

	glUseProgram(m_prog->m_id);

	_render_common(params);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, params.primitive->index_buf->m_id);
	glMultiDrawElements(GL_TRIANGLES, count_lst.data(), GL_UNSIGNED_INT, offset_lst.data(), offset_lst.size());

	glUseProgram(0);
}
