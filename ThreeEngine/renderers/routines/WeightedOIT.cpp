#include <GL/glew.h>
#include <string>
#include "WeightedOIT.h"

static std::string g_vertex =
R"(#version 430
layout (location = 0) out vec2 vUV;
void main()
{
    vec2 grid = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vec2 vpos = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
    gl_Position = vec4(vpos, 1.0, 1.0);
    vUV = vec2(grid.x, grid.y);
}
)";

static std::string g_frag =
R"(#version 430
layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 outColor;
layout (location = 0) uniform sampler2DMS uTex0;
layout (location = 1) uniform sampler2DMS uTex1;
void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);
    float reveal = texelFetch(uTex1, coord, gl_SampleID).x;
    if (reveal>=1.0) discard;
    reveal = 1.0 - reveal;
    vec4 col = texelFetch(uTex0, coord, gl_SampleID);
    col.xyz = col.xyz*reveal/max(col.w, 1e-5);
    col.w = reveal;
    outColor = col;
}
)";

WeightedOIT::WeightedOIT()
{
    m_vert_shader = std::unique_ptr<GLShader>(new GLShader(GL_VERTEX_SHADER, g_vertex.c_str()));
    m_frag_shader = std::unique_ptr<GLShader>(new GLShader(GL_FRAGMENT_SHADER, g_frag.c_str()));
    m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(*m_vert_shader, *m_frag_shader));
}

WeightedOIT::~WeightedOIT()
{
    if (m_fbo_msaa != -1)
        glDeleteFramebuffers(1, &m_fbo_msaa);
    if (m_tex_msaa1 != -1)
        glDeleteTextures(1, &m_tex_msaa1);
    if (m_tex_msaa0 != -1)
        glDeleteTextures(1, &m_tex_msaa0);
}

void WeightedOIT::PreDraw(int width_video, int height_video, unsigned depth_rbo)
{
	if (m_width_video != width_video || m_height_video != height_video)
	{
		if (m_fbo_msaa == -1)
		{
			glGenFramebuffers(1, &m_fbo_msaa);
			glGenTextures(1, &m_tex_msaa0);
			glGenTextures(1, &m_tex_msaa1);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa0);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, width_video, height_video, true);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa0, 0);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa1);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_R8, width_video, height_video, true);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa1, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);

		m_width_video = width_video;
		m_height_video = height_video;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);

	const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);
	glViewport(0, 0, m_width_video, m_height_video);

	float clearColorZero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float clearColorOne[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, clearColorZero);
	glClearBufferfv(GL_COLOR, 1, clearColorOne);

	glEnable(GL_BLEND);
	glBlendFunci(0, GL_ONE, GL_ONE);
	glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	glDepthMask(GL_FALSE);
}

void WeightedOIT::PostDraw()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_SAMPLE_SHADING);

	glDisable(GL_DEPTH_TEST);
	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa0);
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa1);
	glUniform1i(1, 1);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glUseProgram(0);

	glDisable(GL_SAMPLE_SHADING);
}