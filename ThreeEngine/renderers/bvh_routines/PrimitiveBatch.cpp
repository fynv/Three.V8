#include <string>
#include <glm.hpp>
#include <GL/glew.h>
#include "PrimitiveBatch.h"

static std::string g_compute =
R"(#version 430

layout (std140, binding = 0) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (std430, binding = 1) buffer PosIn
{
	vec4 pos_in[];
};

layout (std430, binding = 2) buffer PosOut
{
	vec4 pos_out[];
};

layout (std430, binding = 3) buffer NormIn
{
	vec4 norm_in[];
};

layout (std430, binding = 4) buffer NormOut
{
	vec4 norm_out[];
};


#DEFINES#

#if HAS_COLOR

layout (std430, binding = 5) buffer ColorIn
{
	vec4 color_in[];
};

layout (std430, binding = 6) buffer ColorOut
{
	vec4 color_out[];
};

#endif

#if HAS_UV

layout (std430, binding = 7) buffer UVIn
{
	vec2 uv_in[];
};

layout (std430, binding = 8) buffer UVOut
{
	vec2 uv_out[];
};

#endif


#if HAS_TANGENT

layout (std430, binding = 9) buffer TangentIn
{
	vec4 tangent_in[];
};

layout (std430, binding = 10) buffer TangentOut
{
	vec4 tangent_out[];
};

layout (std430, binding = 11) buffer BitangentIn
{
	vec4 bitangent_in[];
};

layout (std430, binding = 12) buffer BitangentOut
{
	vec4 bitangent_out[];
};

#endif

layout (location = 0) uniform int num_verts;
layout (location = 1) uniform int offset;
layout (local_size_x = 128) in;

void main()
{
	int idx = int(gl_GlobalInvocationID.x);
	if (idx>=num_verts) return;
	
	pos_out[offset + idx] = uModelMat * pos_in[idx];
	norm_out[offset + idx] = uNormalMat * norm_in[idx];

#if HAS_COLOR
	color_out[offset + idx] = color_in[idx];
#endif

#if HAS_UV
	uv_out[offset + idx] = uv_in[idx];
#endif

#if HAS_TANGENT
	tangent_out[offset + idx] = uNormalMat * tangent_in[idx];
	bitangent_out[offset + idx] = uNormalMat * bitangent_in[idx];
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


PrimitiveBatch::PrimitiveBatch(const Options& options) : m_options(options)
{
	std::string s_compute = g_compute;
	std::string defines = "";

	if (options.has_color)
	{
		defines += "#define HAS_COLOR 1\n";
	}
	else
	{
		defines += "#define HAS_COLOR 0\n";
	}

	if (options.has_uv)
	{
		defines += "#define HAS_UV 1\n";
	}
	else
	{
		defines += "#define HAS_UV 0\n";
	}

	if (options.has_tangent)
	{
		defines += "#define HAS_TANGENT 1\n";
	}
	else
	{
		defines += "#define HAS_TANGENT 0\n";
	}

	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}


void PrimitiveBatch::update(const Params& params)
{
	int geo_id = params.primitive_in->geometry.size() - 1;
	const GeometrySet& geo_in = params.primitive_in->geometry[geo_id];
	const GeometrySet& geo_out = params.primitive_out->geometry[0];

	glUseProgram(m_prog->m_id);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_model->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, geo_in.pos_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, geo_out.pos_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, geo_in.normal_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, geo_out.normal_buf->m_id);

	if (m_options.has_color && params.primitive_in->color_buf!=nullptr)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, params.primitive_in->color_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, params.primitive_out->color_buf->m_id);
	}

	if (m_options.has_uv && params.primitive_in->uv_buf!=nullptr)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, params.primitive_in->uv_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, params.primitive_out->uv_buf->m_id);
	}

	if (m_options.has_tangent && geo_in.tangent_buf != nullptr && geo_in.bitangent_buf != nullptr)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, geo_in.tangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, geo_out.tangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, geo_in.bitangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, geo_out.bitangent_buf->m_id);
	}

	glUniform1i(0, params.primitive_in->num_pos);
	glUniform1i(1, params.offset);

	unsigned groups = (params.primitive_in->num_pos + 127) / 128;
	glDispatchCompute(groups, 1, 1);
	glUseProgram(0);
}
