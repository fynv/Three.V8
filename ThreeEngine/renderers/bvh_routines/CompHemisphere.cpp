#include <string>
#include <glm.hpp>
#include <GL/glew.h>
#include "CompHemisphere.h"
#include "renderers/BVHRenderTarget.h"
#include "lights/ProbeRayList.h"
#include "renderers/LightmapRayList.h"
#include "renderers/ReflectionRenderTarget.h"

static std::string g_compute =
R"(#version 430

#DEFINES#

layout (std140, binding = 0) uniform Hemisphere
{
	vec4 uHemisphereSkyColor;
	vec4 uHemisphereGroundColor;
};

layout (binding=0, rgba16f) uniform image2D uOut;

layout(local_size_x = 8, local_size_y = 8) in;

ivec2 g_id_io;
vec3 g_dir;

void render()
{
	float k = g_dir.y * 0.5 + 0.5;
	vec4 outColor =  vec4(mix( uHemisphereGroundColor.xyz, uHemisphereSkyColor.xyz, k), 1.0);
	imageStore(uOut, g_id_io, outColor);
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
	ivec2 size = imageSize(uOut);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;	
	if (id.x>= size.x || id.y >=size.y) return;

	ivec2 screen = ivec2(id.x, id.y);
	vec4 clip = vec4((vec2(screen) + 0.5)/vec2(size)*2.0-1.0, 0.0, 1.0);
	vec4 view = uInvProjMat * clip; view /= view.w;
	vec3 world = vec3(uInvViewMat*view);
	vec3 dir = normalize(world - uEyePos);

	g_id_io = id;
	g_dir = dir;
	render();
}
#elif TO_PROBES

layout (std140, binding = 1) uniform ProbeRayList
{
	mat4 uPRLRotation;
	int uRPLNumProbes;
	int uPRLNumDirections;
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
	int ray_id = local_id.x + local_id.y*8 + group_id.x * 64;	
	g_id_io = ivec2(ray_id, probe_id);

	vec3 sf = sphericalFibonacci(ray_id, uPRLNumDirections);
	vec4 dir = uPRLRotation * vec4(sf, 0.0);
	g_dir = dir.xyz;

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

layout (location = 0) uniform sampler2D uTexNormal;
layout (location = 1) uniform usamplerBuffer uValidList;

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
	g_id_io = ivec2(local_id.x + local_id.y * 8 + group_id.x * 64, group_id.y);
	int idx_texel_out = g_id_io.x/uNumRays + g_id_io.y*uTexelsPerRow;	
	int idx_texel_in = idx_texel_out + uTexelBegin;
	if (idx_texel_in >= uTexelEnd) return;

	int idx_ray = g_id_io.x % uNumRays;

	ivec2 texel_coord = ivec2(texelFetch(uValidList, idx_texel_in).xy);
	vec3 norm = texelFetch(uTexNormal, texel_coord, 0).xyz;
	uint seed = InitRandomSeed(uJitter, idx_texel_out * uNumRays +  idx_ray);
	g_dir = RandomDiffuse(seed, norm);	

	render();	

}
#elif TO_REFLECTION

layout (std140, binding = 1) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

layout (location = 0) uniform sampler2D uTexNormal;

void main()
{
	ivec2 size = imageSize(uOut);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;	
	if (id.x>= size.x || id.y >=size.y) return;

	ivec2 screen = ivec2(id.x, id.y);
	vec4 clip = vec4((vec2(screen) + 0.5)/vec2(size)*2.0-1.0, 0.0, 1.0);
	vec4 view = uInvProjMat * clip; view /= view.w;
	vec3 world = vec3(uInvViewMat*view);
	vec3 dir_in = normalize(world - uEyePos);

	vec4 norm_w = texelFetch(uTexNormal, id, 0);
	if (norm_w.w>0.0)
	{
		vec3 norm = norm_w.xyz/norm_w.w;
		vec3 dir_out = reflect(dir_in, norm);

		g_id_io = id;
		g_dir = dir_out;
		render();
	}
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

CompHemisphere::CompHemisphere(int target_mode) : m_target_mode(target_mode)
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

	if (target_mode == 3)
	{
		defines += "#define TO_REFLECTION 1\n";
	}
	else
	{
		defines += "#define TO_REFLECTION 0\n";
	}

	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}


void CompHemisphere::render(const GLBuffer* constant_camera, const GLBuffer* constant_hemisphere, const BVHRenderTarget* target)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_hemisphere->m_id);

	glBindImageTexture(0, target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, constant_camera->m_id);

	glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);
}

void CompHemisphere::render(const ProbeRayList* prl, const GLBuffer* constant_hemisphere, const BVHRenderTarget* target)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_hemisphere->m_id);

	glBindImageTexture(0, target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, prl->m_constant.m_id);

	glm::ivec2 blocks = { (width + 63) / 64, height };
	glDispatchCompute(blocks.x, blocks.y, 1);


	glUseProgram(0);
}

void CompHemisphere::render(const LightmapRayList* lmrl, const GLBuffer* constant_hemisphere, const BVHRenderTarget* target)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_hemisphere->m_id);

	glBindImageTexture(0, target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, lmrl->m_constant.m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lmrl->source->m_tex_normal->tex_id);
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, lmrl->source->valid_list->tex_id);
	glUniform1i(1, 1);

	int block_x = (target->m_width + 63) / 64;
	glDispatchCompute(block_x, target->m_height, 1);

	glUseProgram(0);
}

void CompHemisphere::render(const GLBuffer* constant_camera, const ReflectionRenderTarget* normal_depth, const GLBuffer* constant_hemisphere, const BVHRenderTarget* target)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_hemisphere->m_id);

	glBindImageTexture(0, target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, constant_camera->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, normal_depth->m_tex_normal->tex_id);
	glUniform1i(0, 0);

	glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);


}

