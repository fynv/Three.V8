#pragma once

#include <memory>
#include <renderers/GLUtils.h>

class GLTexture2D;
class LightmapRenderTarget
{
public:
	LightmapRenderTarget();
	~LightmapRenderTarget();

	int m_width = -1;
	int m_height = -1;

	std::unique_ptr<GLTexture2D> m_tex_position;
	std::unique_ptr<GLTexture2D> m_tex_normal;

	unsigned m_fbo = 0;
	bool update_framebuffer(int width, int height);

	int count_valid;
	std::unique_ptr<TextureBuffer> valid_list;

};

