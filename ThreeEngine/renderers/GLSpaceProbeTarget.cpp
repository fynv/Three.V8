#include <GL/glew.h>
#include "GLSpaceProbeTarget.h"

GLSpaceProbeTarget::GLSpaceProbeTarget()
{
	glGenFramebuffers(1, &m_fbo);
	m_tex_depth = std::unique_ptr<GLTexture2D>(new GLTexture2D);
}

GLSpaceProbeTarget::~GLSpaceProbeTarget()
{
	glDeleteFramebuffers(1, &m_fbo);
}


bool GLSpaceProbeTarget::update_framebuffers(int width, int height)
{
	if (width < 2 || height < 2) return false;
	float rate = sqrtf(65536.0f / ((float)width * (float)height));
	if (rate > 1.0f) rate = 1.0f;
	width = (int)(rate * (float)width);
	height = (int)(rate * (float)height);

	if (m_width != width || m_height != height)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);			

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


void GLSpaceProbeTarget::bind_buffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}
