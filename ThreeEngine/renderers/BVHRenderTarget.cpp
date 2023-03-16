#include <GL/glew.h>
#include "BVHRenderTarget.h"

BVHRenderTarget::BVHRenderTarget()
{
	
}

BVHRenderTarget::~BVHRenderTarget()
{

}

bool BVHRenderTarget::update(int width, int height, bool color, bool depth)
{
	if (m_width != width || m_height != height)
	{
		if (color)
		{
			m_tex_video = std::unique_ptr<GLTexture2D>(new GLTexture2D);
			glBindTexture(GL_TEXTURE_2D, m_tex_video->tex_id);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, width, height);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}

		if (depth)
		{
			m_tex_depth = std::unique_ptr<GLTexture2D>(new GLTexture2D);
			glBindTexture(GL_TEXTURE_2D, m_tex_depth->tex_id);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		m_width = width;
		m_height = height;

		return true;
	}
	return false;
}

void BVHRenderTarget::update_oit_buffers()
{
	m_OITBuffers.update(m_width, m_height);
}

