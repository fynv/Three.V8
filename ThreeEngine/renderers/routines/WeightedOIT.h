#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class WeightedOIT
{
public:
	WeightedOIT(bool msaa);
	~WeightedOIT();

	class Buffers
	{
	public:
		Buffers();
		~Buffers();

		int m_width = -1;
		int m_height = -1;
		unsigned m_tex0 = -1;
		unsigned m_tex1 = -1;
		unsigned m_fbo = -1;

		void update(int width, int height, unsigned depth_tex, bool msaa);
	};

	void PreDraw(Buffers& bufs);
	void PostDraw(Buffers& bufs);

private:
	bool m_msaa;
	std::unique_ptr<GLProgram> m_prog;
};


