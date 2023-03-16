#include <GL/glew.h>
#include <string>
#include <glm.hpp>
#include "CompWeightedOIT.h"
#include "renderers/BVHRenderTarget.h"

static std::string g_compute =
R"(#version 430

layout (location = 0) uniform sampler2D uTex0;
layout (location = 1) uniform sampler2D uTex1;
layout (binding=0, rgba16f) uniform image2D uOut;

layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
	ivec2 size = imageSize(uOut);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;	
	if (id.x>= size.x || id.y >=size.y) return;

	ivec2 coord = ivec2(id.x, id.y);
	float reveal = texelFetch(uTex1, coord, 0).x;
	if (reveal>=1.0) return;
	reveal = 1.0 - reveal;
	vec4 col = texelFetch(uTex0, coord, 0);
	col.xyz = col.xyz*reveal/max(col.w, 1e-5);
    col.w = reveal;

	vec4 base = imageLoad(uOut, id);	
	vec4 col_out = (1.0 - reveal) * base + col;
	imageStore(uOut, id, col_out);
}
)";


CompWeightedOIT::CompWeightedOIT()
{
	GLShader comp_shader(GL_COMPUTE_SHADER, g_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

CompWeightedOIT::~CompWeightedOIT()
{

}

void CompWeightedOIT::Buffers::update(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		m_tex_col = std::unique_ptr<GLTexture2D>(new GLTexture2D);
		m_tex_reveal = std::unique_ptr<GLTexture2D>(new GLTexture2D);

		glBindTexture(GL_TEXTURE_2D, m_tex_col->tex_id);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindTexture(GL_TEXTURE_2D, m_tex_reveal->tex_id);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, width, height);		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_width = width;
		m_height = height;
	}
}


void CompWeightedOIT::PreDraw(Buffers& bufs)
{
	glm::vec4 zeros = { 0.0f, 0.0f, 0.0f, 0.0f };
	glm::vec4 ones = { 1.0f, 1.0f, 1.0f, 1.0f };
	glClearTexImage(bufs.m_tex_col->tex_id, 0, GL_RGBA, GL_FLOAT, &zeros);
	glClearTexImage(bufs.m_tex_reveal->tex_id, 0, GL_RED, GL_FLOAT, &ones);

}


void CompWeightedOIT::PostDraw(const BVHRenderTarget* target)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, target->m_OITBuffers.m_tex_col->tex_id);
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, target->m_OITBuffers.m_tex_reveal->tex_id);
	glUniform1i(1, 1);	

	glBindImageTexture(0, target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);

}

