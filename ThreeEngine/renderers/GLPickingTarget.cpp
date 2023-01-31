#include <GL/glew.h>
#include "GLPickingTarget.h"

GLPickingTarget::GLPickingTarget()
{
	glGenFramebuffers(1, &m_fbo);
	m_tex_idx = std::unique_ptr<GLTexture2D>(new GLTexture2D);
	m_tex_depth = std::unique_ptr<GLTexture2D>(new GLTexture2D);
}


GLPickingTarget::~GLPickingTarget()
{
	glDeleteFramebuffers(1, &m_fbo);	
}

bool GLPickingTarget::update_framebuffers(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		glBindTexture(GL_TEXTURE_2D, m_tex_idx->tex_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_idx->tex_id, 0);

		glBindTexture(GL_TEXTURE_2D, m_tex_depth->tex_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_tex_depth->tex_id, 0);

		m_width = width;
		m_height = height;

		return true;
	}
	return false;
}

void GLPickingTarget::bind_buffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

const GLPickingTarget::IdxInfo& GLPickingTarget::pick_obj(int x, int y)
{
	if (x < 0) x = 0;
	if (x >= m_width) x = m_width - 1;
	if (y < 0) y = 0;
	if (y >= m_height - 1) y = m_height - 1;
	y = m_height - 1 - y;

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	unsigned idx;	
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &idx);	
	return m_idx_info[idx];
}
