#include <cstdio>
#include <GL/glew.h>
#include "LightmapRenderTarget.h"

LightmapRenderTarget::LightmapRenderTarget()
{
	glGenFramebuffers(1, &m_fbo);
}

LightmapRenderTarget::~LightmapRenderTarget()
{
	glDeleteFramebuffers(1, &m_fbo);
}


bool LightmapRenderTarget::update_framebuffer(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		m_tex_position = std::unique_ptr<GLTexture2D>(new GLTexture2D);
		m_tex_normal = std::unique_ptr<GLTexture2D>(new GLTexture2D);

		glBindTexture(GL_TEXTURE_2D, m_tex_position->tex_id);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_position->tex_id, 0);

		glBindTexture(GL_TEXTURE_2D, m_tex_normal->tex_id);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_tex_normal->tex_id, 0);

		m_width = width;
		m_height = height;

		return true;
	}
	return false;
}