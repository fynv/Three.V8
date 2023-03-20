#include <string>
#include <glm.hpp>
#include <GL/glew.h>
#include "CompSkyBox.h"
#include "renderers/BVHRenderTarget.h"
#include "lights/ProbeRayList.h"

static std::string g_compute =
R"(#version 430

#DEFINES#

layout (location = 0) uniform samplerCube uCubeSky;

layout (binding=0, rgba16f) uniform image2D uOut;

layout(local_size_x = 8, local_size_y = 8) in;

ivec2 g_id_io;
vec3 g_dir;

void render()
{
	vec4 outColor = texture(uCubeSky, g_dir);
	imageStore(uOut, g_id_io, outColor);
}


#if TO_CAMERA
layout (std140, binding = 0) uniform Camera
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

layout (std140, binding = 0) uniform ProbeRayList
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

CompSkyBox::CompSkyBox(bool to_probe) : m_to_probe(to_probe)
{
	std::string s_compute = g_compute;

	std::string defines = "";
	if (to_probe)
	{
		defines += "#define TO_CAMERA 0\n";
		defines += "#define TO_PROBES 1\n";
	}
	else
	{
		defines += "#define TO_CAMERA 1\n";
		defines += "#define TO_PROBES 0\n";
	}

	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

void CompSkyBox::render(const GLDynBuffer* constant_camera, const GLCubemap* cubemap, const BVHRenderTarget* target)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->tex_id);
	glUniform1i(0, 0);	

	glBindImageTexture(0, target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_camera->m_id);

	glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);
}

void CompSkyBox::render(const ProbeRayList* prl, const GLCubemap* cubemap, const BVHRenderTarget* target)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->tex_id);
	glUniform1i(0, 0);

	glBindImageTexture(0, target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, prl->m_constant.m_id);

	glm::ivec2 blocks = { (width + 63) / 64, height };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);

}
