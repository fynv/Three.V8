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

static std::string g_frag_noaa =
R"(#version 430
layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 outColor;
layout (location = 0) uniform sampler2D uTex0;
layout (location = 1) uniform sampler2D uTex1;
void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);
    float reveal = texelFetch(uTex1, coord, 0).x;
    if (reveal>=1.0) discard;
    reveal = 1.0 - reveal;
    vec4 col = texelFetch(uTex0, coord, 0);
    col.xyz = col.xyz*reveal/max(col.w, 1e-5);
    col.w = reveal;
    outColor = col;
}
)";


static std::string g_frag_msaa =
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

WeightedOIT::WeightedOIT(bool msaa) : m_msaa(msaa)
{
	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	if (msaa)
	{
		GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag_msaa.c_str());
		m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
	}
	else
	{
		GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag_noaa.c_str());
		m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
	}    
}

WeightedOIT::~WeightedOIT()
{
 
}

WeightedOIT::Buffers::Buffers()
{

}

WeightedOIT::Buffers::~Buffers()
{
	if (m_fbo != -1)
		glDeleteFramebuffers(1, &m_fbo);
	if (m_tex1 != -1)
		glDeleteTextures(1, &m_tex1);
	if (m_tex0 != -1)
		glDeleteTextures(1, &m_tex0);
}

void WeightedOIT::Buffers::update(int width, int height, unsigned depth_tex, bool msaa)
{
	if (m_width != width || m_height != height)
	{
		if (m_fbo == -1)
		{
			glGenFramebuffers(1, &m_fbo);
			glGenTextures(1, &m_tex0);
			glGenTextures(1, &m_tex1);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		if (msaa)
		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex0);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, width, height, true);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_tex0, 0);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex1);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_R8, width, height, true);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, m_tex1, 0);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth_tex, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_tex0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex0, 0);

			glBindTexture(GL_TEXTURE_2D, m_tex1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_tex1, 0);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
		}


		m_width = width;
		m_height = height;
	}
}

void WeightedOIT::PreDraw(Buffers& bufs)
{
	glBindFramebuffer(GL_FRAMEBUFFER, bufs.m_fbo);

	const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);
	glViewport(0, 0, bufs.m_width, bufs.m_height);

	float clearColorZero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float clearColorOne[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, clearColorZero);
	glClearBufferfv(GL_COLOR, 1, clearColorOne);

	glEnable(GL_BLEND);
	glBlendFunci(0, GL_ONE, GL_ONE);
	glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	glDepthMask(GL_FALSE);
}

void WeightedOIT::PostDraw(Buffers& bufs)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if (m_msaa)
	{
		glEnable(GL_SAMPLE_SHADING);
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	if (m_msaa)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, bufs.m_tex0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, bufs.m_tex0);
	}
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	if (m_msaa)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, bufs.m_tex1);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, bufs.m_tex1);
	}
	glUniform1i(1, 1);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	if (m_msaa)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);

	if (m_msaa)
	{
		glDisable(GL_SAMPLE_SHADING);
	}
}