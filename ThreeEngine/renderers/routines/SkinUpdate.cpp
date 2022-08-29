#include <string>
#include <glm.hpp>
#include <GL/glew.h>
#include "SkinUpdate.h"

static std::string g_compute =
R"(#version 430

layout (std430, binding = 0) buffer Mat
{
	mat4 rela_mats[];
};

layout (std430, binding = 1) buffer Joints
{
	uvec4 joints[];
};

layout (std430, binding = 2) buffer Weights
{
	vec4 weights[];
};

layout (std430, binding = 3) buffer PosRest
{
	vec4 pos_rests[];
};

layout (std430, binding = 4) buffer PosOut
{
	vec4 pos_out[];
};

layout (std430, binding = 5) buffer NormRest
{
	vec4 norm_rests[];
};

layout (std430, binding = 6) buffer NormOut
{
	vec4 norm_out[];
};

#DEFINES#

#if HAS_TANGENT

layout (std430, binding = 7) buffer TangentRest
{
	vec4 tangent_rests[];
};

layout (std430, binding = 8) buffer TangentOut
{
	vec4 tangent_out[];
};

layout (std430, binding = 9) buffer BitangentRest
{
	vec4 bitangent_rests[];
};

layout (std430, binding = 10) buffer BitangentOut
{
	vec4 bitangent_out[];
};

#endif

layout (location = 0) uniform int num_verts;
layout (local_size_x = 128) in;

void main()
{
	int idx = int(gl_GlobalInvocationID.x);
	if (idx>=num_verts) return;
	uvec4 joint = joints[idx];
	vec4 weight = weights[idx];
	
	mat4 mat = mat4(0.0);
	
	mat += weight.x * rela_mats[joint.x];
	mat += weight.y * rela_mats[joint.y];
	mat += weight.z * rela_mats[joint.z];
	mat += weight.w * rela_mats[joint.w];

	pos_out[idx] = mat * pos_rests[idx];
	norm_out[idx] = mat * norm_rests[idx];

#if HAS_TANGENT
	tangent_out[idx] = mat * tangent_rests[idx];
	bitangent_out[idx] = mat * bitangent_rests[idx];
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


SkinUpdate::SkinUpdate(bool has_tangent) : m_has_tangent(has_tangent)
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

	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}


void SkinUpdate::update(const Params& params)
{
	int geo_id = params.primitive->num_targets > 0 ? 1 : 0;
	const GeometrySet& geo_in = params.primitive->geometry[geo_id];
	const GeometrySet& geo_out = params.primitive->geometry[geo_id+1];		

	glUseProgram(m_prog->m_id);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, params.skin->buf_rela_mat->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, params.primitive->joints_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, params.primitive->weights_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, geo_in.pos_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, geo_out.pos_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, geo_in.normal_buf->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, geo_out.normal_buf->m_id);

	if (m_has_tangent)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, geo_in.tangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, geo_out.tangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, geo_in.bitangent_buf->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, geo_out.bitangent_buf->m_id);
	}

	glUniform1i(0, params.primitive->num_pos);

	unsigned groups = (params.primitive->num_pos + 127) / 128;
	glDispatchCompute(groups, 1, 1);
	glUseProgram(0);
}