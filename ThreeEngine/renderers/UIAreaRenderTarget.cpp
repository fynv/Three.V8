#include <cstdio>
#include <GL/glew.h>
#include <renderers/GLUtils.h>
#include "UIAreaRenderTarget.h"

UIAreaRenderTarget::UIAreaRenderTarget()
{
	glGenFramebuffers(1, &m_fbo);
	m_tex = std::unique_ptr<GLTexture2D>(new GLTexture2D);
}

bool UIAreaRenderTarget::update_framebuffers(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
			
		glBindTexture(GL_TEXTURE_2D, m_tex->tex_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex->tex_id, 0);

		m_width = width;
		m_height = height;

		return true;
	}
	return false;
}

void UIAreaRenderTarget::bind_buffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}
