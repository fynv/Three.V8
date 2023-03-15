#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class BVHRenderTarget;
class CompWeightedOIT
{
public:
	CompWeightedOIT();
	~CompWeightedOIT();

	class Buffers
	{
	public:
		int m_width = -1;
		int m_height = -1;
		std::unique_ptr<GLTexture2D> m_tex_col;
		std::unique_ptr<GLTexture2D> m_tex_reveal;

		void update(int width, int height);
	};

	void PreDraw(Buffers& bufs);
	void PostDraw(const BVHRenderTarget* target);

private:
	std::unique_ptr<GLProgram> m_prog;
};


