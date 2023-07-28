#include <GL/glew.h>
#include "UpdateTriangles.h"

static std::string g_compute =
R"(#version 430

#DEFINES#

layout (std430, binding = 0) buffer BVHIndices
{
	int bBVHIndices[];
};

layout (std430, binding = 1) buffer BVHTriangles
{
	vec4 bBVHTriangles[];
};

layout (std430, binding = 2) buffer PrimPositions
{
	vec4 bPrimPositions[];
};

layout (location = 0) uniform int num_triangles;

#if HAS_INDICES
layout (location = 1) uniform usamplerBuffer uTexFaces;
#endif


layout(local_size_x = 64) in;

void main()
{	
	int idx = int(gl_GlobalInvocationID.x);
	if (idx>=num_triangles) return;

	int prim_face_idx = bBVHIndices[idx];
	ivec3 vert_idx;

#if HAS_INDICES
	vert_idx.x = int(texelFetch(uTexFaces, prim_face_idx*3).x);
	vert_idx.y = int(texelFetch(uTexFaces, prim_face_idx*3 + 1).x);
	vert_idx.z = int(texelFetch(uTexFaces, prim_face_idx*3 + 2).x);
#else
	vert_idx.x = prim_face_idx*3;
	vert_idx.y = prim_face_idx*3+1;
	vert_idx.z = prim_face_idx*3+2;
#endif
	
	vec3 pos0 = bPrimPositions[vert_idx.x].xyz;
	vec3 pos1 = bPrimPositions[vert_idx.y].xyz;
	vec3 pos2 = bPrimPositions[vert_idx.z].xyz;

	vec3 edge1 = pos1 - pos0;
	vec3 edge2 = pos2 - pos0;

	bBVHTriangles[idx*3] = vec4(pos0, 1.0);
	bBVHTriangles[idx*3+1] = vec4(edge1, 0.0);
	bBVHTriangles[idx*3+2] = vec4(edge2, 0.0);
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

UpdateTriangles::UpdateTriangles(bool has_indices) : m_has_indices(has_indices)
{
	std::string defines = "";
	if (has_indices)
	{		
		defines += "#define HAS_INDICES 1\n";
	}
	else
	{	
		defines += "#define HAS_INDICES 0\n";
	}

	std::string s_compute = g_compute;

	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));

}

void UpdateTriangles::update(int num_triangles, GLBuffer* bvh_indices, GLBuffer* bvh_triangles,
	GLBuffer* prim_positions, unsigned tex_prim_indices)
{
	glUseProgram(m_prog->m_id);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bvh_indices->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bvh_triangles->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, prim_positions->m_id);

	glUniform1i(0, num_triangles);

	if (m_has_indices)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, tex_prim_indices);
		glUniform1i(1, 0);
	}

	int blocks = (num_triangles + 63) / 64;

	glDispatchCompute(blocks, 1, 1);

	glUseProgram(0);
	
}
