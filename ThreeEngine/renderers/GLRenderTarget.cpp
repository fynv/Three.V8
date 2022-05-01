#include <cstdio>
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
	else if (!msaa)
	{
		printf("Using default buffer without MSAA, Order-Indepent-Transparency will fail.\n");
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
	{
		glDeleteFramebuffers(1, &m_fbo_video);
		if (m_tex_video != (unsigned)(-1))
			glDeleteTextures(1, &m_tex_video);
		if (m_rbo_video != (unsigned)(-1))
			glDeleteRenderbuffers(1, &m_rbo_video);
	}

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

void GLRenderTarget::update_oit_buffers()
{
	if (m_fbo_msaa == (unsigned)(-1))
	{
		m_OITBuffers.update(m_width, m_height, m_rbo_video, false);	
	}
	else
	{
		m_OITBuffers.update(m_width, m_height, m_rbo_msaa, true);		
	}
}


void GLRenderTarget::bind_buffer()
{
	if (m_fbo_msaa!=(unsigned)(-1))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_video);
	}	
}


void GLRenderTarget::resolve_msaa()
{
	if (m_fbo_msaa != (unsigned)(-1))
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_video);
		glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_video);
	}
}

void GLRenderTarget::blit_buffer(int width_wnd, int height_wnd, int margin)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDisable(GL_FRAMEBUFFER_SRGB);

	glViewport(0, 0, width_wnd, height_wnd);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	int client_w = width_wnd - margin * 2;
	int client_h = height_wnd - margin * 2;

	if (client_w < m_width || client_h < m_height)
	{
		// scale down
		int dst_w = m_width * client_h / m_height;
		if (dst_w <= client_w)
		{
			int dst_offset = (client_w - dst_w) / 2;
			glBlitFramebuffer(0, 0, m_width, m_height, margin + dst_offset, margin, margin + dst_offset + dst_w, margin + client_h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		}
		else
		{
			int dst_h = m_height * client_w / m_width;
			int dst_offset = (client_h - dst_h) / 2;
			glBlitFramebuffer(0, 0, m_width, m_height, margin, margin + dst_offset, margin + client_w, margin + dst_offset + dst_h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
	}
	else
	{
		// center
		int offset_x = (width_wnd - m_width) / 2;
		int offset_y = (height_wnd - m_height) / 2;
		glBlitFramebuffer(0, 0, m_width, m_height, offset_x, offset_y, offset_x + m_width, offset_y + m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}