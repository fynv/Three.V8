#include <GL/glew.h>
#include "models/ModelComponents.h"
#include "NormalAndDepth.h"
#include "cameras/Camera.h"


static std::string g_vertex =
R"(#version 430

#DEFINES#

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};


layout (std140, binding = 1) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (location = 0) out vec3 vNorm;


#if HAS_NORMAL_MAP
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

layout (location = 1) out vec2 vUV;
layout (location = 2) out vec3 vTangent;
layout (location = 3) out vec3 vBitangent;
#endif

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	gl_Position = uProjMat*(uViewMat*wolrd_pos);
	vec4 world_norm = uNormalMat * vec4(aNorm, 0.0);
	vNorm = world_norm.xyz;

#if HAS_NORMAL_MAP
	vUV = aUV;

	vec4 world_tangent = uModelMat * vec4(aTangent, 0.0);
	vTangent =  world_tangent.xyz;

	vec4 world_bitangent = uModelMat * vec4(aBitangent, 0.0);
	vBitangent =  world_bitangent.xyz;
#endif
}
)";


static std::string g_frag =
R"(#version 430

#DEFINES#

layout (location = 0) in vec3 vNorm;

layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

layout (std140, binding = 2) uniform Material
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

#if HAS_NORMAL_MAP
layout (location = 0) uniform sampler2D uTexNormal;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec3 vBitangent;
#endif

layout (location = 0) out vec4 out0;

void main()
{
	vec3 norm = normalize(vNorm);

	if (uDoubleSided!=0 && !gl_FrontFacing)
	{		
		norm = -norm;
	}

#if HAS_NORMAL_MAP
	if (length(vTangent)>0.0 && length(vBitangent)>0.0)
	{
		vec3 T = normalize(vTangent);
		vec3 B = normalize(vBitangent);
		vec3 bump =  texture(uTexNormal, vUV).xyz;
		bump = (2.0 * bump - 1.0) * vec3(uNormalScale.x, uNormalScale.y, 1.0);
		norm = normalize(bump.x*T + bump.y*B + bump.z*norm);
	}
#endif	

	out0 = vec4((norm + 1.0)*0.5, 1.0);	
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

NormalAndDepth::NormalAndDepth(bool has_normal_map) : m_has_normal_map(has_normal_map)
{
	std::string s_vertex, s_frag;

	s_vertex = g_vertex;
	s_frag = g_frag;

	std::string defines = "";
	if (has_normal_map)
	{
		defines += "#define HAS_NORMAL_MAP 1\n";
	}
	else
	{
		defines += "#define HAS_NORMAL_MAP 0\n";
	}

	replace(s_vertex, "#DEFINES#", defines.c_str());
	replace(s_frag, "#DEFINES#", defines.c_str());

	GLShader vert_shader(GL_VERTEX_SHADER, s_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, s_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}


void NormalAndDepth::render(const RenderParams& params)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	glFrontFace(params.camera->reflector != nullptr ? GL_CW : GL_CCW);
	if (material.doubleSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.camera->m_constant.m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_model->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, material.constant_material.m_id);

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, geo.normal_buf->m_id);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	if (m_has_normal_map)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->uv_buf->m_id);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, geo.tangent_buf->m_id);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, geo.bitangent_buf->m_id);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(4);

		const GLTexture2D& tex = *params.tex_list[material.tex_idx_normalMap];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(0, 0);
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


void NormalAndDepth::render_batched(const RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	if (material.doubleSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.camera->m_constant.m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_model->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, material.constant_material.m_id);

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, geo.normal_buf->m_id);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	if (m_has_normal_map)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->uv_buf->m_id);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, geo.tangent_buf->m_id);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, geo.bitangent_buf->m_id);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(4);

		const GLTexture2D& tex = *params.tex_list[material.tex_idx_normalMap];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(0, 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, params.primitive->index_buf->m_id);
	glMultiDrawElements(GL_TRIANGLES, count_lst.data(), GL_UNSIGNED_INT, offset_lst.data(), offset_lst.size());

	glUseProgram(0);

}






