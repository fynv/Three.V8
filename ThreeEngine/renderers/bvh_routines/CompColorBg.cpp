#include <string>
#include <glm.hpp>
#include <GL/glew.h>
#include "CompColorBg.h"
#include "renderers/BVHRenderTarget.h"
#include "renderers/ReflectionRenderTarget.h"

static std::string g_compute =
R"(#version 430

layout (location = 0) uniform vec3 uColor;
layout (binding=0, rgba16f) uniform image2D uOut;

layout(local_size_x = 8, local_size_y = 8) in;

layout (location = 1) uniform sampler2D uTexNormal;

void main()
{
	ivec2 size = imageSize(uOut);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;	
	if (id.x>= size.x || id.y >=size.y) return;

	vec4 norm = texelFetch(uTexNormal, id, 0);
	if (norm.w>0.0)
	{
		imageStore(uOut, id, vec4(uColor, 1.0));
	}
}
)";


CompColorBg::CompColorBg()
{
	GLShader comp_shader(GL_COMPUTE_SHADER, g_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}


void CompColorBg::render(const ReflectionRenderTarget* normal_depth, const glm::vec3& color, const BVHRenderTarget* target)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);

	glUniform3fv(0, 1, (float*)&color);
	glBindImageTexture(0, target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, normal_depth->m_tex_normal->tex_id);
	glUniform1i(1, 0);

	glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);


}