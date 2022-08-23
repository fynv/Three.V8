#include <string>
#include <glm.hpp>
#include <GL/glew.h>
#include "MorphUpdate.h"

static std::string g_compute =
R"(#version 430

layout (std430, binding = 0) buffer Coefficients
{
	float coefs[];
};

layout (std430, binding = 1) buffer PosBase
{
	vec4 pos_base[];
};

layout (std430, binding = 2) buffer PosDelta
{
	vec4 pos_delta[];
};

layout (std430, binding = 3) buffer PosOut
{
	vec4 pos_out[];
};

layout (std430, binding = 4) buffer NormBase
{
	vec4 norm_base[];
};

layout (std430, binding = 5) buffer NormDelta
{
	vec4 norm_delta[];
};

layout (std430, binding = 6) buffer NormOut
{
	vec4 norm_out[];
};

#DEFINES#

#if HAS_TANGENT
layout (std430, binding = 7) buffer TangentBase
{
	vec4 tangent_base[];
};

layout (std430, binding = 8) buffer TangentDelta
{
	vec4 tangent_delta[];
};

layout (std430, binding = 9) buffer TangentOut
{
	vec4 tangent_out[];
};

layout (std430, binding = 10) buffer BitangentBase
{
	vec4 bitangent_base[];
};

layout (std430, binding = 11) buffer BitangentDelta
{
	vec4 bitangent_delta[];
};

layout (std430, binding = 12) buffer BitangentOut
{
	vec4 bitangent_out[];
};
#endif

#if SPARSE
layout (std430, binding = 13) buffer Nonzero
{
	int nonzero[];
};
#endif

layout (location = 0) uniform int num_verts;
layout (location = 1) uniform int num_deltas;
layout(local_size_x = 128) in;

void main()
{
	int idx = int(gl_GlobalInvocationID.x);
	if (idx>=num_verts) return;
	vec4 pos = pos_base[idx];
	vec4 norm = norm_base[idx];
#if HAS_TANGENT
	vec4 tangent = tangent_base[idx];
	vec4 bitangent = bitangent_base[idx];
#endif

#if SPARSE
	bool nonzero = (nonzero[idx]!=0);
#else
	bool nonzero = true;
#endif
	if (nonzero)
	{
		for (int i = 0; i< num_deltas; i++)
		{
			float coef = clamp(coefs[i], 0.0, 1.0);
			if (coef==0.0) continue;
			{
				vec4 delta = pos_delta[i*num_verts + idx];
				pos += delta * coef;
			}
			{
				vec4 delta = norm_delta[i*num_verts + idx];
				norm += delta * coef;
			}
#if HAS_TANGENT
			{
				vec4 delta = tangent_delta[i*num_verts + idx];
				tangent += delta * coef;
			}
			{
				vec4 delta = bitangent_delta[i*num_verts + idx];
				bitangent += delta * coef;
			}
#endif
		}
	}
	pos_out[idx] = pos;
	norm_out[idx] = norm;
#if HAS_TANGENT
	tangent_out[idx] = tangent;
	bitangent_out[idx] = bitangent;
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

MorphUpdate::MorphUpdate(bool has_tangent, bool sparse) 
	: m_has_tangent(has_tangent)
	, m_sparse(sparse)
{
	std::string s_compute = g_compute;
	std::string defines = "";

	if (has_tangent)
	{
		defines += "#define HAS_TANGENT 1\n";
	}
	else
	{
		defines += "#define HAS_TANGENT 0\n";
	}

	if (sparse)
	{
		defines += "#define SPARSE 1\n";
	}
	else
	{
		defines += "#define SPARSE 0\n";
	}

	replace(s_compute, "#DEFINES#", defines.c_str());

	m_comp_shader = std::unique_ptr<GLShader>(new GLShader(GL_COMPUTE_SHADER, s_compute.c_str()));
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(*m_comp_shader));
}

void MorphUpdate::update(const Params& params)
{
	const GeometrySet& geo_in = params.primitive->geometry[0];
	const GeometrySet& geo_out = params.primitive->geometry[1];

	glUseProgram(m_prog->m_id);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, params.buf_weights->m_id);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, geo_in.pos_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, params.primitive->targets.pos_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, geo_out.pos_buf->m_id);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, geo_in.normal_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, params.primitive->targets.normal_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, geo_out.normal_buf->m_id);

	if (m_has_tangent)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, geo_in.tangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, params.primitive->targets.tangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, geo_out.tangent_buf->m_id);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, geo_in.bitangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, params.primitive->targets.bitangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, geo_out.bitangent_buf->m_id);
	}

	if (m_sparse)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, params.primitive->none_zero_buf->m_id);
	}

	glUniform1i(0, params.primitive->num_pos);
	glUniform1i(1, params.primitive->num_targets);

	unsigned groups = (params.primitive->num_pos + 127) / 128;
	glDispatchCompute(groups, 1, 1);
	glUseProgram(0);
}