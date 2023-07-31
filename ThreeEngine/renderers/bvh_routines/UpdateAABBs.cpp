#include <GL/glew.h>
#include "UpdateAABBs.h"

static std::string g_compute =
R"(#version 430

#DEFINES#

layout (std430, binding = 0) buffer BVHIndices
{
	int bBVHIndices[];
};

layout (std430, binding = 1) buffer BVHNodes
{
	vec4 bBVHNodes[];
};

layout (std430, binding = 2) buffer NodeIndices
{
	int bNodeIndices[];
};

layout (std430, binding = 3) buffer PrimPositions
{
	vec4 bPrimPositions[];
};

layout (location = 0) uniform int num_nodes;

#if HAS_INDICES
layout (location = 1) uniform usamplerBuffer uTexFaces;
#endif

layout(local_size_x = 64) in;

uint extract_byte(uint x, uint i) 
{
	return (x >> (i * 8)) & 0xff;
}

void main()
{	
	int t_idx = int(gl_GlobalInvocationID.x);
	if (t_idx>=num_nodes) return;

	int idx = bNodeIndices[t_idx];

	vec4 node_1 = bBVHNodes[idx * 5 + 1];

	uint base_index_child = floatBitsToUint(node_1.x); 
	uint base_index_triangle = floatBitsToUint(node_1.y); 

	uint meta4[2];
	meta4[0] = floatBitsToUint(node_1.z);
	meta4[1] = floatBitsToUint(node_1.w);

	vec3 min_pos[8];
	vec3 max_pos[8];
	vec3 min_common = vec3(3.402823466e+38);
	vec3 max_common = vec3(-3.402823466e+38);

	int rel_id = 0;
	for (int i=0; i<8; i++)
	{
		uint meta = extract_byte(meta4[i/4], i%4);
		bool is_inner = ((meta & (meta << 1)) & 0x10) != 0;
		if (is_inner)
		{
			int child_idx = rel_id + int(base_index_child);
			rel_id++;

			vec4 child_node_0 = bBVHNodes[child_idx * 5];
			vec4 child_node_1 = bBVHNodes[child_idx * 5 + 1];
			vec4 child_node_2 = bBVHNodes[child_idx * 5 + 2];
			vec4 child_node_3 = bBVHNodes[child_idx * 5 + 3];
			vec4 child_node_4 = bBVHNodes[child_idx * 5 + 4];

			vec3 p = child_node_0.xyz;
			uint u_e_imask = floatBitsToUint(child_node_0.w);
			uvec3 u_e = uvec3(extract_byte(u_e_imask, 0), extract_byte(u_e_imask, 1), extract_byte(u_e_imask, 2));
			vec3 e = vec3(uintBitsToFloat(u_e.x << 23), uintBitsToFloat(u_e.y << 23), uintBitsToFloat(u_e.z << 23));

			min_pos[i] = p;
			max_pos[i] = p;

			uint child_meta4[2];
			child_meta4[0] = floatBitsToUint(child_node_1.z);
			child_meta4[1] = floatBitsToUint(child_node_1.w);

			uint quantized_max_x4[2];
			uint quantized_max_y4[2];
			uint quantized_max_z4[2];
			quantized_max_x4[0] = floatBitsToUint(child_node_2.z);
			quantized_max_x4[1] = floatBitsToUint(child_node_2.w);
			quantized_max_y4[0] = floatBitsToUint(child_node_3.z);
			quantized_max_y4[1] = floatBitsToUint(child_node_3.w);
			quantized_max_z4[0] = floatBitsToUint(child_node_4.z);
			quantized_max_z4[1] = floatBitsToUint(child_node_4.w);

			for (int k = 0; k < 8; k++)
			{
				uint child_meta = extract_byte(child_meta4[k/4], k%4);
				bool child_is_inner = ((child_meta & (child_meta << 1)) & 0x10) != 0; 
				if (!child_is_inner)
				{
					int count = bitCount(child_meta >> 5);
					if (count < 1) continue;
				}

				uvec3 quantized_max = uvec3( 
					extract_byte(quantized_max_x4[k/4], k%4),
					extract_byte(quantized_max_y4[k/4], k%4),
					extract_byte(quantized_max_z4[k/4], k%4));

				vec3 pos_max = p + vec3(quantized_max)*e;
				max_pos[i] = max(max_pos[i], pos_max);
			}
		}
		else
		{
			int tri_id = int((meta & 0x1F) + base_index_triangle);
			int count = bitCount(meta >> 5);

			min_pos[i] = vec3(3.402823466e+38);
			max_pos[i] = vec3(-3.402823466e+38);

			if (count>0)
			{
				for (int k = 0; k < count; k++)
				{
					int prim_face_idx = bBVHIndices[tri_id + k];
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
					min_pos[i] = min(min_pos[i], pos0);
					max_pos[i] = max(max_pos[i], pos0);

					vec3 pos1 = bPrimPositions[vert_idx.y].xyz;
					min_pos[i] = min(min_pos[i], pos1);
					max_pos[i] = max(max_pos[i], pos1);

					vec3 pos2 = bPrimPositions[vert_idx.z].xyz;
					min_pos[i] = min(min_pos[i], pos2);
					max_pos[i] = max(max_pos[i], pos2);
				}
				float eps = 0.001;
				while (max_pos[i].x - min_pos[i].x < eps)
				{
					max_pos[i].x += eps;
					min_pos[i].x -= eps;
					eps *= 2.0f;
				}
				eps = 0.001;
				while (max_pos[i].y - min_pos[i].y < eps)
				{
					max_pos[i].y += eps;
					min_pos[i].y -= eps;
					eps *= 2.0f;
				}
				eps = 0.001;
				while (max_pos[i].z - min_pos[i].z < eps)
				{
					max_pos[i].z += eps;
					min_pos[i].z -= eps;
					eps *= 2.0f;
				}
			
			}				
		}
		min_common = min(min_common, min_pos[i]);
		max_common = max(max_common, max_pos[i]);
	}	

	float denom = 1.0 / 255.0;
	
	vec3 e = exp2(ceil(log2((max_common - min_common)*denom)));

	uvec3 u_e = uvec3(floatBitsToUint(e.x)>>23,  floatBitsToUint(e.y)>>23, floatBitsToUint(e.z)>>23);
	{
		vec4 node_0 = bBVHNodes[idx * 5];
		node_0.xyz = min_common;
		uint u_e_imask = floatBitsToUint(node_0.w);
		u_e_imask = (u_e_imask & 0xFF000000) | (u_e.z << 16) | (u_e.y <<8) | u_e.x;
		node_0.w = uintBitsToFloat(u_e_imask);
		bBVHNodes[idx * 5] = node_0;
	}

	vec3 one_over_e = 1.0 / e;
	
	vec4 node_2 = bBVHNodes[idx * 5 + 2];
	vec4 node_3 = bBVHNodes[idx * 5 + 3];
	vec4 node_4 = bBVHNodes[idx * 5 + 4];

	uint quantized_min_x4[2];
	uint quantized_min_y4[2];
	uint quantized_min_z4[2];
	uint quantized_max_x4[2];
	uint quantized_max_y4[2];
	uint quantized_max_z4[2];
	
	quantized_min_x4[0] = floatBitsToUint(node_2.x);
	quantized_min_x4[1] = floatBitsToUint(node_2.y);
	quantized_max_x4[0] = floatBitsToUint(node_2.z);
	quantized_max_x4[1] = floatBitsToUint(node_2.w);
	quantized_min_y4[0] = floatBitsToUint(node_3.x);
	quantized_min_y4[1] = floatBitsToUint(node_3.y);
	quantized_max_y4[0] = floatBitsToUint(node_3.z);
	quantized_max_y4[1] = floatBitsToUint(node_3.w);
	quantized_min_z4[0] = floatBitsToUint(node_4.x);
	quantized_min_z4[1] = floatBitsToUint(node_4.y);
	quantized_max_z4[0] = floatBitsToUint(node_4.z);
	quantized_max_z4[1] = floatBitsToUint(node_4.w);

	for (int i = 0; i < 8; i++)
	{
		uint meta = extract_byte(meta4[i/4], i%4);
		bool is_inner = ((meta & (meta << 1)) & 0x10) != 0;
		if (!is_inner)
		{			
			int count = bitCount(meta >> 5);
			if (count<1) continue;
		}
		
		uvec3 quantized_min = uvec3(floor((min_pos[i] - min_common) * one_over_e));
		uvec3 quantized_max = uvec3(ceil((max_pos[i] - min_common) * one_over_e));

		quantized_min_x4[i/4] = (quantized_min_x4[i/4] & (~(0xFF<<((i%4)*8)))) | ((quantized_min.x & 0xFF) << ((i%4)*8));
		quantized_min_y4[i/4] = (quantized_min_y4[i/4] & (~(0xFF<<((i%4)*8)))) | ((quantized_min.y & 0xFF) << ((i%4)*8));
		quantized_min_z4[i/4] = (quantized_min_z4[i/4] & (~(0xFF<<((i%4)*8)))) | ((quantized_min.z & 0xFF) << ((i%4)*8));

		quantized_max_x4[i/4] = (quantized_max_x4[i/4] & (~(0xFF<<((i%4)*8)))) | ((quantized_max.x & 0xFF) << ((i%4)*8));
		quantized_max_y4[i/4] = (quantized_max_y4[i/4] & (~(0xFF<<((i%4)*8)))) | ((quantized_max.y & 0xFF) << ((i%4)*8));
		quantized_max_z4[i/4] = (quantized_max_z4[i/4] & (~(0xFF<<((i%4)*8)))) | ((quantized_max.z & 0xFF) << ((i%4)*8));
	}

	node_2.x = uintBitsToFloat(quantized_min_x4[0]);
	node_2.y = uintBitsToFloat(quantized_min_x4[1]);
	node_2.z = uintBitsToFloat(quantized_max_x4[0]);
	node_2.w = uintBitsToFloat(quantized_max_x4[1]);
	node_3.x = uintBitsToFloat(quantized_min_y4[0]);
	node_3.y = uintBitsToFloat(quantized_min_y4[1]);
	node_3.z = uintBitsToFloat(quantized_max_y4[0]);
	node_3.w = uintBitsToFloat(quantized_max_y4[1]);
	node_4.x = uintBitsToFloat(quantized_min_z4[0]);
	node_4.y = uintBitsToFloat(quantized_min_z4[1]);
	node_4.z = uintBitsToFloat(quantized_max_z4[0]);
	node_4.w = uintBitsToFloat(quantized_max_z4[1]);
	
	bBVHNodes[idx * 5 + 2] = node_2;
	bBVHNodes[idx * 5 + 3] = node_3;
	bBVHNodes[idx * 5 + 4] = node_4;
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

UpdateAABBs::UpdateAABBs(bool has_indices) : m_has_indices(has_indices)
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

void UpdateAABBs::update(int num_nodes, GLBuffer* bvh_indices, GLBuffer* bvh_nodes,
	GLBuffer* node_indices, GLBuffer* prim_positions, unsigned tex_prim_indices)
{
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glUseProgram(m_prog->m_id);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bvh_indices->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bvh_nodes->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, node_indices->m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, prim_positions->m_id);

	glUniform1i(0, num_nodes);

	if (m_has_indices)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, tex_prim_indices);
		glUniform1i(1, 0);
	}

	int blocks = (num_nodes + 63) / 64;

	glDispatchCompute(blocks, 1, 1);

	glUseProgram(0);

}


