#include <cstdio>
#include <GL/glew.h>
#include "GLRenderTarget.h"
#include "CubeRenderTarget.h"
#include "utils/Image.h"

GLRenderTarget::GLRenderTarget(bool default_buffer, bool msaa)
{	
	if (!default_buffer)
	{
		glGenFramebuffers(1, &m_fbo_video);
		m_tex_video = std::unique_ptr<GLTexture2D>(new GLTexture2D);
		if (!msaa)
		{
			m_tex_depth = std::unique_ptr<GLTexture2D>(new GLTexture2D);
		}
	}
	else if (!msaa)
	{
		printf("Using default buffer without MSAA, Order-Indepent-Transparency will fail.\n");
	}

	if (msaa)
	{
		glGenFramebuffers(1, &m_fbo_msaa);
		m_tex_msaa = std::unique_ptr<GLTexture2D>(new GLTexture2D);
		m_tex_depth = std::unique_ptr<GLTexture2D>(new GLTexture2D);		
	}
}

GLRenderTarget::GLRenderTarget(CubeRenderTarget* cube_target, int idx)
{
	m_cube_target = cube_target;
	m_cube_face_idx = idx;

	glGenFramebuffers(1, &m_fbo_video);
	m_tex_depth = std::unique_ptr<GLTexture2D>(new GLTexture2D);

}

GLRenderTarget::~GLRenderTarget()
{
	if (m_fbo_video != 0)
		glDeleteFramebuffers(1, &m_fbo_video);	

	if (m_fbo_msaa != (unsigned)(-1))
		glDeleteFramebuffers(1, &m_fbo_msaa);
}


bool GLRenderTarget::update_framebuffers(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		if (m_fbo_video != 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_video);

			if (m_cube_target == nullptr)
			{
				glBindTexture(GL_TEXTURE_2D, m_tex_video->tex_id);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glBindTexture(GL_TEXTURE_2D, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_video->tex_id, 0);
			}
			else
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_cube_face_idx, m_cube_target->m_cube_map->tex_id,0);
			}

			if (m_fbo_msaa == (unsigned)(-1))			
			{
				glBindTexture(GL_TEXTURE_2D, m_tex_depth->tex_id);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glBindTexture(GL_TEXTURE_2D, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_tex_depth->tex_id, 0);
			}
		}

		if (m_fbo_msaa != (unsigned)(-1))
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa->tex_id);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_SRGB8_ALPHA8, width, height, true);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa->tex_id, 0);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_depth->tex_id);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT32F, width, height, true);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_tex_depth->tex_id, 0);
		}

		m_width = width;
		m_height = height;

		return true;
	}
	return false;
}

void GLRenderTarget::update_oit_buffers()
{
	m_OITBuffers.update(m_width, m_height, m_tex_depth->tex_id, msaa());
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

void GLRenderTarget::GetImage(Image& image)
{
	size_t buf_size = (size_t)m_width * (size_t)m_height * 4;

	glBindTexture(GL_TEXTURE_2D, m_tex_video->tex_id);
	image.m_width = m_width;
	image.m_height = m_height;
	free(image.m_buffer);
	image.m_buffer = (uint8_t*)malloc(buf_size);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.m_buffer);	

	uint8_t* line_buffer = (uint8_t*)malloc((size_t)m_width * 4);
	for (int i = 0; i < m_height / 2; i++)
	{
		int j = m_height - 1 - i;
		memcpy(line_buffer, image.m_buffer + (size_t)m_width * 4 * i, (size_t)m_width * 4);
		memcpy(image.m_buffer + (size_t)m_width * 4 * i, image.m_buffer + (size_t)m_width * 4 * j, (size_t)m_width * 4);
		memcpy(image.m_buffer + (size_t)m_width * 4 * j, line_buffer, (size_t)m_width * 4);
	}
	free(line_buffer);

	glBindTexture(GL_TEXTURE_2D, 0);
}

