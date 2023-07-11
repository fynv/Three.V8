#include <string>
#include <GL/glew.h>
#include "ReflectionCopy.h"
#include "cameras/Camera.h"

static std::string g_comp = 
R"(#version 430

layout (std140, binding = 0) uniform Scissor
{
	ivec4 uScissor;
};

layout(location = 0) uniform sampler2D tex_src;
layout(binding = 0, rgba16f) uniform image2D tex_dst;

layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
	ivec2 size = imageSize(tex_dst);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;
	if (id.x>=size.x || id.y>=size.y) return;
	id = clamp(id, uScissor.xy,  uScissor.xy + uScissor.zw-1);
	vec4 color = texelFetch(tex_src, id, 0);
	imageStore(tex_dst, id, color);
}

)";

ReflectionCopy::ReflectionCopy()
{
	GLShader comp_shader(GL_COMPUTE_SHADER, g_comp.c_str());	
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

void ReflectionCopy::copy(unsigned tex_dst, unsigned tex_refl, int width, int height, const Camera* camera)
{
	glUseProgram(m_prog->m_id);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera->m_constant_scissor.m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_refl);
	glUniform1i(0, 0);

	glBindImageTexture(0, tex_dst, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	glDispatchCompute((width + 7) / 8, (height + 7) / 8, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}


