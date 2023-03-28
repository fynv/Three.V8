#include <GL/glew.h>
#include "CubeRenderTarget.h"
#include "GLUtils.h"
#include "GLRenderTarget.h"
#include "utils/Image.h"
#include "utils/HDRImage.h"

CubeRenderTarget::CubeRenderTarget()
{
	m_cube_map = std::unique_ptr<GLCubemap>(new GLCubemap);
	for (int i = 0; i < 6; i++)
	{
		m_faces[i] = std::unique_ptr<GLRenderTarget>(new GLRenderTarget(this, i));
	}
}

CubeRenderTarget::~CubeRenderTarget()
{

}

bool CubeRenderTarget::update_framebuffers(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_cube_map->tex_id);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		for (int i = 0; i < 6; i++)
		{
			m_faces[i]->update_framebuffers(width, height);
		}
		m_width = width;
		m_height = height;
		return true;
	}
	return false;
}


void CubeRenderTarget::GetCubeImage(CubeImage& image)
{
	size_t buf_size = (size_t)m_width * (size_t)m_height * 4;

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cube_map->tex_id);
	for (int i = 0; i < 6; i++)
	{
		Image& img_i = image.images[i];
		img_i.m_width = m_width;
		img_i.m_height = m_height;
		free(img_i.m_buffer);		
		img_i.m_buffer = (uint8_t*)malloc(buf_size);		
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_i.m_buffer);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}


void CubeRenderTarget::GetHDRCubeImage(HDRCubeImage& image)
{
	size_t buf_size = (size_t)m_width * (size_t)m_height * 3 * sizeof(float);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cube_map->tex_id);
	for (int i = 0; i < 6; i++)
	{
		HDRImage& img_i = image.images[i];
		img_i.m_width = m_width;
		img_i.m_height = m_height;
		free(img_i.m_buffer);
		img_i.m_buffer = (float*)malloc(buf_size);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, GL_FLOAT, img_i.m_buffer);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}