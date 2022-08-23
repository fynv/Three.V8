#pragma once

#include "renderers/GLRenderTarget.h"
#include "renderers/routines/DrawTexture.h"

struct NVGcontext;
class UIManager;
class Image;
class GLUIRenderer
{
public:
	GLUIRenderer();
	~GLUIRenderer();

	bool HasFont(const char* name);
	void CreateFont(const char* name, const char* filename);
	void CreateFont(const char* name, unsigned char* data, size_t size);

	int CreateImage(Image* img);
	void DeleteImage(int img_id);

	void render(UIManager& UI, int width_wnd, int height_wnd);

private:
	NVGcontext* vg;
	DrawTexture blitter;

	double time_last_render = -1.0;
};

