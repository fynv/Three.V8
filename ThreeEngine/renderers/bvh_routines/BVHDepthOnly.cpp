#include <GL/glew.h>
#include "models/ModelComponents.h"
#include "BVHDepthOnly.h"
#include "renderers/BVHRenderTarget.h"
#include "lights/ProbeRayList.h"
#include "renderers/LightmapRayList.h"

static std::string g_compute =
R"(#version 430

uint ray_get_octant_inv4(in vec3 ray_direction)
{
	return (ray_direction.x < 0.0 ? 0 : 0x04040404) |
		(ray_direction.y < 0.0 ? 0 : 0x02020202) |
		(ray_direction.z < 0.0 ? 0 : 0x01010101);
}

struct BVH8Node
{
	vec4 node_0;
	vec4 node_1;
	vec4 node_2;
	vec4 node_3;
	vec4 node_4; 
};

layout (location = 0) uniform samplerBuffer uTexBVH8;
layout (location = 1) uniform samplerBuffer uTexTriangles;
layout (location = 2) uniform isamplerBuffer uTexIndices;

#DEFINES#

layout (std140, binding = 0) uniform Material
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


struct Ray
{
	vec3 origin;
	float tmin;
	vec3 direction;	
	float tmax;
};

uint extract_byte(uint x, uint i) 
{
	return (x >> (i * 8)) & 0xff;
}

uint sign_extend_s8x4(uint x) 
{
	return ((x >> 7) & 0x01010101) * 0xff;
}

uint bvh8_node_intersect(in Ray ray, uint oct_inv4, in BVH8Node node)
{
	vec3 p = node.node_0.xyz;
	
	uint e_imask = floatBitsToUint(node.node_0.w);
	uint e_x = extract_byte(e_imask, 0);
	uint e_y = extract_byte(e_imask, 1);
	uint e_z = extract_byte(e_imask, 2);

	vec3 adjusted_ray_direction_inv = vec3(
		uintBitsToFloat(e_x << 23) / ray.direction.x,
		uintBitsToFloat(e_y << 23) / ray.direction.y,
		uintBitsToFloat(e_z << 23) / ray.direction.z
	);

	vec3 adjusted_ray_origin = (p - ray.origin) / ray.direction;

	uint hit_mask = 0;

	for (int i = 0; i < 2; i++) 
	{
		uint meta4 = floatBitsToUint(i == 0 ? node.node_1.z : node.node_1.w);
		
		uint is_inner4   = (meta4 & (meta4 << 1)) & 0x10101010;
		uint inner_mask4 = sign_extend_s8x4(is_inner4 << 3);
		uint bit_index4  = (meta4 ^ (oct_inv4 & inner_mask4)) & 0x1f1f1f1f;
		uint child_bits4 = (meta4 >> 5) & 0x07070707;

		// Select near and far planes based on ray octant
		uint q_lo_x = floatBitsToUint(i == 0 ? node.node_2.x : node.node_2.y);
		uint q_hi_x = floatBitsToUint(i == 0 ? node.node_2.z : node.node_2.w);

		uint q_lo_y = floatBitsToUint(i == 0 ? node.node_3.x : node.node_3.y);
		uint q_hi_y = floatBitsToUint(i == 0 ? node.node_3.z : node.node_3.w);

		uint q_lo_z = floatBitsToUint(i == 0 ? node.node_4.x : node.node_4.y);
		uint q_hi_z = floatBitsToUint(i == 0 ? node.node_4.z : node.node_4.w);

		uint x_min = ray.direction.x < 0.0 ? q_hi_x : q_lo_x;
		uint x_max = ray.direction.x < 0.0 ? q_lo_x : q_hi_x;

		uint y_min = ray.direction.y < 0.0 ? q_hi_y : q_lo_y;
		uint y_max = ray.direction.y < 0.0 ? q_lo_y : q_hi_y;

		uint z_min = ray.direction.z < 0.0 ? q_hi_z : q_lo_z;
		uint z_max = ray.direction.z < 0.0 ? q_lo_z : q_hi_z;

		for (int j = 0; j < 4; j++) 
		{
			// Extract j-th byte
			vec3 tmin3 = vec3(float(extract_byte(x_min, j)), float(extract_byte(y_min, j)), float(extract_byte(z_min, j)));
			vec3 tmax3 = vec3(float(extract_byte(x_max, j)), float(extract_byte(y_max, j)), float(extract_byte(z_max, j)));

			// Account for grid origin and scale
			tmin3 = tmin3 * adjusted_ray_direction_inv + adjusted_ray_origin;
			tmax3 = tmax3 * adjusted_ray_direction_inv + adjusted_ray_origin;

			float tmin = max(max(tmin3.x, tmin3.y), max(tmin3.z, ray.tmin));
			float tmax = min(min(tmax3.x, tmax3.y), min(tmax3.z, ray.tmax));

			bool intersected = tmin < tmax;
			if (intersected) 
			{
				uint child_bits = extract_byte(child_bits4, j);
				uint bit_index  = extract_byte(bit_index4,  j);
				hit_mask |= child_bits << bit_index;
			}
		}
	}

	return hit_mask;
}

bool triangle_intersect(int triangle_id, in Ray ray, out float t, out float u, out float v)
{
	vec3 pos0 = texelFetch(uTexTriangles, triangle_id*3).xyz;
	vec3 edge1 = texelFetch(uTexTriangles, triangle_id*3 + 1).xyz;
	vec3 edge2 = texelFetch(uTexTriangles, triangle_id*3 + 2).xyz;
	
	vec3 h = cross(ray.direction, edge2);
	float a = dot(edge1, h);

	//if (a==0.0 ||  (uDoubleSided==0 && a<0.0)) return false;
	if (a==0.0) return false;
	
	float f = 1.0 / a;
	vec3 s = ray.origin - pos0;
	u = f * dot(s, h);

	if (u < 0.0 || u > 1.0) return false;
	
	vec3 q = cross(s, edge1);
	v = f * dot(ray.direction, q);

	if (v < 0.0 || (u + v)> 1.0) return false;
	t = f * dot(edge2, q);

	if (t <= ray.tmin) return false;	
	return t <= ray.tmax;
}


#define BVH_STACK_SIZE 32
#define SHARED_STACK_SIZE 8
#define LOCAL_STACK_SIZE (BVH_STACK_SIZE - SHARED_STACK_SIZE)
shared uvec2 shared_stack_bvh8[SHARED_STACK_SIZE*64];

#define SHARED_STACK_INDEX(offset) ((gl_LocalInvocationID.y * SHARED_STACK_SIZE + offset) * 32 + gl_LocalInvocationID.x)

void stack_push(inout uvec2 stack[LOCAL_STACK_SIZE], inout int stack_size, in uvec2 item) {	

	if (stack_size < SHARED_STACK_SIZE) 
	{
		shared_stack_bvh8[SHARED_STACK_INDEX(stack_size)] = item;
	} 
	else 
	{
		stack[stack_size - SHARED_STACK_SIZE] = item;
	}
	stack_size++;
}

uvec2 stack_pop(in uvec2 stack[LOCAL_STACK_SIZE], inout int stack_size) 
{
	stack_size--;
	if (stack_size < SHARED_STACK_SIZE) 
	{
		return shared_stack_bvh8[SHARED_STACK_INDEX(stack_size)];
	} 
	else 
	{
		return stack[stack_size - SHARED_STACK_SIZE];
	}
}

Ray g_ray;

void intersect()
{
	uvec2 stack[LOCAL_STACK_SIZE]; 
	int stack_size = 0;

	uint oct_inv4 = ray_get_octant_inv4(g_ray.direction);
	uvec2 current_group = uvec2(0, 0x80000000);
	
	while (stack_size > 0 || current_group.y!=0)
	{
		uvec2 triangle_group;
		if ((current_group.y & 0xff000000)!=0)
		{
			uint hits_imask = current_group.y;
			int child_index_offset = findMSB(hits_imask);
			uint child_index_base   = current_group.x;

			// Remove n from current_group;
			current_group.y &= ~(1 << child_index_offset);

			// If the node group is not yet empty, push it on the stack
			if ((current_group.y & 0xff000000)!=0) 
			{
				stack_push(stack, stack_size, current_group);
			}

			uint slot_index     = (child_index_offset - 24) ^ (oct_inv4 & 0xff);
			uint relative_index = bitCount(hits_imask & ~(0xffffffff << slot_index));

			uint child_node_index = child_index_base + relative_index;
			BVH8Node node;
			node.node_0 = texelFetch(uTexBVH8, int(child_node_index*5));
			node.node_1 = texelFetch(uTexBVH8, int(child_node_index*5 + 1));
			node.node_2 = texelFetch(uTexBVH8, int(child_node_index*5 + 2));
			node.node_3 = texelFetch(uTexBVH8, int(child_node_index*5 + 3));
			node.node_4 = texelFetch(uTexBVH8, int(child_node_index*5 + 4));
			uint hitmask = bvh8_node_intersect(g_ray, oct_inv4, node);

			uint imask = extract_byte(floatBitsToUint(node.node_0.w), 3);			

			current_group.x = floatBitsToUint(node.node_1.x); // Child    base offset
			triangle_group.x = floatBitsToUint(node.node_1.y); // Triangle base offset

			current_group.y = (hitmask & 0xff000000) | imask;
			triangle_group.y = (hitmask & 0x00ffffff);
		}
		else 
		{
			triangle_group = current_group;
			current_group  = uvec2(0);
		}

		while (triangle_group.y != 0)
		{
			int triangle_index = findMSB(triangle_group.y);
			triangle_group.y &= ~(1 << triangle_index);

			int tri_idx = int(triangle_group.x + triangle_index);
			float t,u,v;
			if (triangle_intersect(tri_idx, g_ray, t, u, v))
			{				
				g_ray.tmax = t;			
			}
		}			

		if ((current_group.y & 0xff000000) == 0) 
		{
			if (stack_size == 0) break;
			current_group = stack_pop(stack, stack_size);			
		}
	}
}

layout (binding=0, r32f) uniform image2D uDepth;
layout(local_size_x = 32, local_size_y = 2) in;

ivec2 g_id_io;
vec3 g_origin;
vec3 g_dir;
float g_tmin;
float g_tmax;

void render()
{
	float tmax = imageLoad(uDepth, g_id_io).x;
	g_tmax = min(tmax, g_tmax);

	g_ray.origin = g_origin;
	g_ray.direction = g_dir;
	g_ray.tmin = g_tmin;
	g_ray.tmax = g_tmax;

	intersect();	
	
	if (g_ray.tmax < tmax)
	{		
		imageStore(uDepth, g_id_io, vec4(g_ray.tmax));
	}	
}

#if TO_CAMERA
layout (std140, binding = 1) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

void main()
{
	ivec2 size = imageSize(uDepth);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;	
	if (id.x>= size.x || id.y >=size.y) return;	

	ivec2 screen = ivec2(id.x, id.y);
	vec4 clip0 = vec4((vec2(screen) + 0.5)/vec2(size)*2.0-1.0, -1.0, 1.0);
	vec4 clip1 = vec4((vec2(screen) + 0.5)/vec2(size)*2.0-1.0, 1.0, 1.0);
	vec4 view0 = uInvProjMat * clip0; view0 /= view0.w;
	vec4 view1 = uInvProjMat * clip1; view1 /= view1.w;
	vec3 world0 = vec3(uInvViewMat*view0);
	vec3 world1 = vec3(uInvViewMat*view1);	
	vec3 dir = normalize(world0 - uEyePos);

	g_id_io = id;
	g_origin = uEyePos;
	g_dir = dir;
	g_tmin = length(world0 - uEyePos);
	g_tmax = length(world1 - uEyePos);
	
	render();
}
#elif TO_PROBES

layout (std140, binding = 1) uniform ProbeRayList
{
	mat4 uPRLRotation;
	int uRPLNumProbes;
	int uPRLNumDirections;	
};

layout (std430, binding = 2) buffer ProbePositions
{
	vec4 uProbePos[];
};

#define PI 3.14159265359

vec3 sphericalFibonacci(float i, float n)
{
	const float PHI = sqrt(5.0) * 0.5 + 0.5;
	float m = i * (PHI - 1.0);
	float frac_m = m - floor(m);
	float phi = 2.0 *  PI * frac_m;
	float cosTheta = 1.0 - (2.0 * i + 1.0) * (1.0 / n);
    float sinTheta = sqrt(clamp(1.0 - cosTheta * cosTheta, 0.0, 1.0));
	return vec3(cos(phi)*sinTheta, sin(phi)*sinTheta, cosTheta);	
}

void main()
{	
	ivec2 local_id = ivec3(gl_LocalInvocationID).xy;	
	ivec2 group_id = ivec3(gl_WorkGroupID).xy;
	int probe_id = group_id.y;
	int ray_id = local_id.x + local_id.y*32 + group_id.x * 64;	
	g_id_io = ivec2(ray_id, probe_id);
	g_origin = uProbePos[probe_id].xyz;

	vec3 sf = sphericalFibonacci(ray_id, uPRLNumDirections);
	vec4 dir = uPRLRotation * vec4(sf, 0.0);
	g_dir = dir.xyz;

	g_tmin = 0.0;	
	g_tmax = 3.402823466e+38;
	
	render();
}
#elif TO_LIGHTMAP
layout (std140, binding = 1) uniform LightmapRayList
{
	int uTexelBegin;
	int uTexelEnd;
	int uNumRays;
	int uTexelsPerRow;
	int uNumRows;
	int uJitter;
};

layout (location = 3) uniform sampler2D uTexPosition;
layout (location = 4) uniform sampler2D uTexNormal;
layout (location = 5) uniform usamplerBuffer uValidList;

#define PI 3.14159265359

uint InitRandomSeed(uint val0, uint val1)
{
	uint v0 = val0, v1 = val1, s0 = 0u;

	for (uint n = 0u; n < 16u; n++)
	{
		s0 += 0x9e3779b9u;
		v0 += ((v1 << 4) + 0xa341316cu) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4u);
		v1 += ((v0 << 4) + 0xad90777du) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761eu);
	}

	return v0;
}

uint RandomInt(inout uint seed)
{
    return (seed = 1664525u * seed + 1013904223u);
}

float RandomFloat(inout uint seed)
{
	return (float(RandomInt(seed) & 0x00FFFFFFu) / float(0x01000000));
}

vec3 RandomDirection(inout uint seed)
{
	float z = RandomFloat(seed) * 2.0 - 1.0;
	float xy = sqrt(1.0 - z*z);
	float alpha = RandomFloat(seed) * PI * 2.0;
	return vec3(xy * cos(alpha), xy * sin(alpha), z);
}

vec3 RandomDiffuse(inout uint seed, in vec3 base_dir)
{
	vec3 dir = RandomDirection(seed);
	float d = dot(dir, base_dir);
	vec3 c = d * base_dir;
	vec3 s = dir - c;
	float z2 = clamp(abs(d), 0.0, 1.0);
	float xy = sqrt(1.0 - z2);	
	vec3 s_dir =  sqrt(z2) * base_dir;
	if (length(s)>0.0)
	{		
		s_dir += xy * normalize(s);
	}
	return s_dir;
}

void main()
{
	ivec2 local_id = ivec3(gl_LocalInvocationID).xy;	
	ivec2 group_id = ivec3(gl_WorkGroupID).xy;
	g_id_io = ivec2(local_id.x + local_id.y * 32 + group_id.x * 64, group_id.y);
	int idx_texel_out = g_id_io.x/uNumRays + g_id_io.y*uTexelsPerRow;	
	int idx_texel_in = idx_texel_out + uTexelBegin;
	if (idx_texel_in >= uTexelEnd) return;

	int idx_ray = g_id_io.x % uNumRays;
	ivec2 texel_coord = ivec2(texelFetch(uValidList, idx_texel_in).xy);	
	g_origin = texelFetch(uTexPosition, texel_coord, 0).xyz;
	vec3 norm = texelFetch(uTexNormal, texel_coord, 0).xyz;
	uint seed = InitRandomSeed(uJitter, idx_texel_out * uNumRays +  idx_ray);
	g_dir = RandomDiffuse(seed, norm);	

	g_tmin = 0.001;	
	g_tmax = 3.402823466e+38;
	
	render();
}
#endif
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

BVHDepthOnly::BVHDepthOnly(int target_mode) : m_target_mode(target_mode)
{
	std::string s_compute = g_compute;
	
	std::string defines = "";
	if (target_mode == 0)
	{
		defines += "#define TO_CAMERA 1\n";
	}
	else
	{
		defines += "#define TO_CAMERA 0\n";
	}

	if (target_mode == 1)
	{
		defines += "#define TO_PROBES 1\n";
	}
	else
	{
		defines += "#define TO_PROBES 0\n";
	}

	if (target_mode == 2)
	{
		defines += "#define TO_LIGHTMAP 1\n";
	}
	else
	{
		defines += "#define TO_LIGHTMAP 0\n";
	}

	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

void BVHDepthOnly::render(const RenderParams& params)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];	

	const CWBVH* bvh = params.primitive->cwbvh.get();
	const BVHRenderTarget* target = params.target;

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, bvh->m_tex_bvh8.tex_id);
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, bvh->m_tex_triangles.tex_id);
	glUniform1i(1, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, bvh->m_tex_indices.tex_id);
	glUniform1i(2, 2);
	
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, material.constant_material.m_id);

	glBindImageTexture(0, target->m_tex_depth->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);

	if (m_target_mode == 0)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_camera->m_id);		

		glm::ivec2 blocks = { (width + 31) / 32, (height + 1) / 2 };
		glDispatchCompute(blocks.x, blocks.y, 1);
	}
	else if (m_target_mode == 1)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.prl->m_constant.m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, params.prl->buf_positions->m_id);

		glm::ivec2 blocks = { (width + 63) / 64, height };
		glDispatchCompute(blocks.x, blocks.y, 1);
	}
	else if (m_target_mode == 2)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.lmrl->m_constant.m_id);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, params.lmrl->source->m_tex_position->tex_id);
		glUniform1i(3, 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, params.lmrl->source->m_tex_normal->tex_id);
		glUniform1i(4, 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_BUFFER, params.lmrl->source->valid_list->tex_id);
		glUniform1i(5, 5);

		glm::ivec2 blocks = { (width + 63) / 64, height };
		glDispatchCompute(blocks.x, blocks.y, 1);

	}

	glUseProgram(0);
}