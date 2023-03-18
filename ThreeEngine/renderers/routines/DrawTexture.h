#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class DrawTexture
{
public:
	DrawTexture(bool premult=false, bool flipY = false, bool auto_contrast = false);
	void render(unsigned tex_id, int x, int y, int width, int height, bool blending = false, float alpha = 1.0f);

private:
	bool m_auto_contrast = false;
	std::unique_ptr<GLProgram> m_prog;

};
