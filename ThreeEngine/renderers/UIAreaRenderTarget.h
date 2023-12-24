#pragma once

#include <memory>

class GLTexture2D;
class UIAreaRenderTarget
{
public:
	UIAreaRenderTarget();

	int m_width = -1;
	int m_height = -1;

	std::unique_ptr<GLTexture2D> m_tex;	
	unsigned m_fbo = 0;	
	bool update_framebuffers(int width, int height);

	void bind_buffer();
	

};
