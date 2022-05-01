#include <GL/glew.h>
#include "GLRenderTarget.h"

GLRenderTarget::GLRenderTarget(bool default_buffer, bool msaa)
{
	if (!default_buffer)
	{
		glGenFramebuffers(1, &m_fbo_video);
		glGenTextures(1, &m_tex_video);
		if (!msaa)
		{
			glGenRenderbuffers(1, &m_rbo_video);
		}
	}

	if (msaa)
	{
		glGenFramebuffers(1, &m_fbo_msaa);
		glGenTextures(1, &m_tex_msaa);
		glGenRenderbuffers(1, &m_rbo_msaa);
	}
}

GLRenderTarget::~GLRenderTarget()
{
	if (m_fbo_video != 0)
		glDeleteFramebuffers(1, &m_fbo_video);
	if (m_tex_video != (unsigned)(-1))
		glDeleteTextures(1, &m_tex_video);
	if (m_rbo_video != (unsigned)(-1))
		glDeleteRenderbuffers(1, &m_rbo_video);

	if (m_fbo_msaa != (unsigned)(-1))
		glDeleteFramebuffers(1, &m_fbo_msaa);
	if (m_tex_msaa != (unsigned)(-1))
		glDeleteTextures(1, &m_tex_msaa);
	if (m_rbo_msaa != (unsigned)(-1))
		glDeleteRenderbuffers(1, &m_rbo_msaa);
}


void GLRenderTarget::update_framebuffers(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		if (m_fbo_video != 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_video);

			glBindTexture(GL_TEXTURE_2D, m_tex_video);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_video, 0);

			if (m_rbo_video != (unsigned)(-1))
			{
				glBindRenderbuffer(GL_RENDERBUFFER, m_rbo_video);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo_video);
			}
		}

		if (m_fbo_msaa != (unsigned)(-1))
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_SRGB8_ALPHA8, width, height, true);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa, 0);

			glBindRenderbuffer(GL_RENDERBUFFER, m_rbo_msaa);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo_msaa);
		}

		m_width = width;
		m_height = height;
	}
}


