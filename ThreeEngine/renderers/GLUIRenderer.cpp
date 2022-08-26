#include <cmath>
#include <GL/glew.h>
#include "GLUIRenderer.h"
#include "utils/Image.h"
#include "gui/UIManager.h"
#include "gui/UIArea.h"
#include "gui/UIElement.h"
#include "gui/UIBlock.h"
#include "gui/UIPanel.h"
#include "gui/UIButton.h" 
#include "gui/UIScrollViewer.h"
#include "gui/UIText.h"
#include "gui/UITextBlock.h"
#include "gui/UIImage.h"
#include "gui/UILineEdit.h"

#include "gui/UI3DViewer.h"

#include "utils/Utils.h"

#include "nanovg.h"
#define NANOVG_GLES3_IMPLEMENTATION
#include "nanovg_gl.h"


static char* cpToUTF8(int cp, char* str)
{
	int n = 0;
	if (cp < 0x80) n = 1;
	else if (cp < 0x800) n = 2;
	else if (cp < 0x10000) n = 3;
	else if (cp < 0x200000) n = 4;
	else if (cp < 0x4000000) n = 5;
	else if (cp <= 0x7fffffff) n = 6;
	str[n] = '\0';
	switch (n) {
	case 6: str[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
	case 5: str[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
	case 4: str[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
	case 3: str[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
	case 2: str[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
	case 1: str[0] = cp;
	}
	return str;
}



GLUIRenderer::GLUIRenderer() : blitter(true)
{
	vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
}

GLUIRenderer::~GLUIRenderer()
{
	nvgDeleteGLES3(vg);
}

bool GLUIRenderer::HasFont(const char* name)
{
	return nvgFindFont(vg, name) != -1;
}

void GLUIRenderer::CreateFont(const char* name, const char* filename)
{
	nvgCreateFont(vg, name, filename);
}

void GLUIRenderer::CreateFont(const char* name, unsigned char* data, size_t size)
{
	unsigned char* mem = (unsigned char*)malloc(size);
	memcpy(mem, data, size);
	nvgCreateFontMem(vg, name, mem, (int)size, 1);
}

int GLUIRenderer::CreateImage(Image* img)
{
	return nvgCreateImageRGBA(vg, img->width(), img->height(), 0, img->data());
}

void GLUIRenderer::DeleteImage(int img_id)
{
	nvgDeleteImage(vg, img_id);
}

class AreaRenderer
{
public:
	UIArea* area;
	float scale;
	int x0, y0, x1, y1;
	int w, h;
	float x_offset, y_offset;	

	AreaRenderer(UIArea* area) : area(area)
	{
		scale = area->scale;
		x0 = (int)floorf(area->origin.x * scale);
		y0 = (int)floorf(area->origin.y * scale);

		x1 = (int)ceilf((area->origin.x + area->size.x) * scale);
		y1 = (int)ceilf((area->origin.y + area->size.y) * scale);

		w = x1 - x0;
		h = y1 - y0;
		x_offset = area->origin.x * scale - (float)x0;
		y_offset = area->origin.y * scale - (float)y0;
		
	}

	void Render(NVGcontext* vg)
	{
		if (w != area->render_target.m_width || h != area->render_target.m_height)
		{
			area->render_target.update_framebuffers(w, h);
		}

		area->render_target.bind_buffer();
		glDisable(GL_FRAMEBUFFER_SRGB);		

		glViewport(0, 0, w, h);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		nvgBeginFrame(vg, (float)w, (float)h, 1.0f);

		size_t num_elements = area->elements.size();
		for (size_t i = 0; i < num_elements; i++)
		{
			UIElement* elem = area->elements[i];
			elem->update_origin_trans();
			elem->appearance_changed = false;

			nvgResetScissor(vg);

			UIBlock* block = elem->block;
			while (block!=nullptr)
			{			
				if (block->scissor_enabled)
				{
					glm::vec2 origin;
					convert_pos(block->origin_trans, origin);
					glm::vec2 scissor_origin = origin + block->scissor_offset() * scale;
					glm::vec2 scissor_size = block->scissor_size() * scale;
					nvgIntersectScissor(vg, scissor_origin.x, scissor_origin.y, scissor_size.x, scissor_size.y);
				}				
				block = block->block;
			}

			do
			{
				UIBlock* block = dynamic_cast<UIBlock*>(elem);
				if (block != nullptr)
				{
					do
					{
						UIPanel* panel = dynamic_cast<UIPanel*>(block);
						if (panel != nullptr)
						{
							RenderPanel(vg, panel);
							break;
						}

						UIButton* btn = dynamic_cast<UIButton*>(block);
						if (btn != nullptr)
						{
							RenderButton(vg, btn);
							break;
						}

						UIScrollViewer* sview = dynamic_cast<UIScrollViewer*>(block);
						if (sview != nullptr)
						{
							RenderScrollViewer(vg, sview);
							break;
						}


					} while (false);
					break;
				}

				UILineEdit* ui_line_edit = dynamic_cast<UILineEdit*>(elem);
				if (ui_line_edit != nullptr)
				{
					RenderLineEdit(vg, ui_line_edit);
					break;
				}

				UIText* ui_text = dynamic_cast<UIText*>(elem);
				if (ui_text != nullptr)
				{
					RenderText(vg, ui_text);
					break;
				}

				UITextBlock* ui_text_block = dynamic_cast<UITextBlock*>(elem);
				if (ui_text_block != nullptr)
				{
					RenderTextBlock(vg, ui_text_block);
					break;
				}

				UIImage* ui_image = dynamic_cast<UIImage*>(elem);
				if (ui_image != nullptr)
				{
					RenderImage(vg, ui_image);
					break;
				}

			} while (false);

		}

		nvgEndFrame(vg);

		glEnable(GL_DEPTH_TEST);
	
	}

	void RenderViews(DrawTexture* blitter)
	{
		size_t num_views = area->viewers.size();
		for (size_t i = 0; i < num_views; i++)
		{
			UI3DViewer* view = area->viewers[i];
			view->update_origin_trans();

			glm::vec2 origin, size;
			convert_pos(view->origin_trans, origin);
			convert_size(view->size, size);

			glm::ivec2 upper_left, lower_right;
			upper_left.x = (int)(origin.x + 0.5f);
			upper_left.y = (int)(origin.y + 0.5f);
			lower_right.x = (int)(origin.x + size.x + 1.5f);
			lower_right.y = (int)(origin.y + size.y + 1.5f);

			glm::ivec2 i_size = lower_right - upper_left;
			bool size_changed = view->render_target.update_framebuffers(i_size.x, i_size.y);

			if (view->render_callback != nullptr)
			{
				view->render_callback(i_size.x, i_size.y, size_changed, view->render_data);
			}

			view->render_target.resolve_msaa();

			bool scissor = false;
			glm::ivec2 scissor_upper_Left = { 0, 0 };
			glm::ivec2 scissor_lower_right = { w, h };

			UIBlock* block = view->block;
			while (block != nullptr)
			{
				UIScrollViewer* sview = dynamic_cast<UIScrollViewer*>(block);
				if (sview != nullptr)
				{
					glm::vec2 sorigin;
					convert_pos(sview->origin_trans, sorigin);
					glm::vec2 scissor_origin = sorigin + sview->scissor_offset() * scale;
					glm::vec2 scissor_size = sview->scissor_size() * scale;

					glm::ivec2 s_upper_left, s_lower_right;
					s_upper_left.x = (int)(scissor_origin.x + 0.5f);
					s_upper_left.y = (int)(scissor_origin.y + 0.5f);
					s_lower_right.x = (int)(scissor_origin.x + scissor_size.x + 1.5f);
					s_lower_right.y = (int)(scissor_origin.y + scissor_size.y + 1.5f);

					if (s_upper_left.x > scissor_upper_Left.x)
					{
						scissor_upper_Left.x = s_upper_left.x;
						scissor = true;
					}

					if (s_upper_left.y > scissor_upper_Left.y)
					{
						scissor_upper_Left.y = s_upper_left.y;
						scissor = true;
					}

					if (s_lower_right.x < scissor_lower_right.x)
					{
						scissor_lower_right.x = s_lower_right.x;
						scissor = true;
					}

					if (s_lower_right.y < scissor_lower_right.y)
					{
						scissor_lower_right.y = s_lower_right.y;
						scissor = true;
					}
				}
				block = block->block;
			}

			area->render_target.bind_buffer();
			glEnable(GL_FRAMEBUFFER_SRGB);

			if (scissor)
			{
				glEnable(GL_SCISSOR_TEST);
				glScissor(scissor_upper_Left.x, scissor_upper_Left.y, scissor_lower_right.x - scissor_upper_Left.x, scissor_lower_right.y - scissor_upper_Left.y);
			}
			else
			{
				glDisable(GL_SCISSOR_TEST);
			}

			blitter->render(view->render_target.m_tex_video, upper_left.x, h - lower_right.y, i_size.x, i_size.y, false);

		}
		glDisable(GL_SCISSOR_TEST);
	}

private:
	void convert_pos(const glm::vec2& pos_local, glm::vec2& pos_area)
	{
		pos_area.x = pos_local.x * scale + x_offset;
		pos_area.y = pos_local.y * scale + y_offset;
	}

	void convert_size(const glm::vec2& size_local, glm::vec2& size_area)
	{
		size_area.x = size_local.x * scale;
		size_area.y = size_local.y * scale;
	}

	void RenderPanel(NVGcontext* vg, UIPanel* panel)
	{
		glm::vec2 origin, size;
		convert_pos(panel->origin_trans, origin);
		convert_size(panel->size, size);
		float cornerRadius = panel->cornerRadius * scale;
		float strokeWidth = panel->strokeWidth * scale;		
		NVGcolor color_bg = nvgRGBA(panel->colorBg.r, panel->colorBg.g, panel->colorBg.b, panel->colorBg.a);
		NVGcolor color_stroke = nvgRGBA(panel->colorStroke.r, panel->colorStroke.g, panel->colorStroke.b, panel->colorStroke.a);

		NVGpaint bg = nvgLinearGradient(vg, origin.x, origin.y, origin.x, origin.y + size.y, nvgRGBA(255, 255, 255, 32), nvgRGBA(0, 0, 0, 32));
		nvgBeginPath(vg);
		nvgRoundedRect(vg, origin.x + strokeWidth, origin.y + strokeWidth, size.x - strokeWidth*2.0f, size.y - strokeWidth*2.0f, cornerRadius - strokeWidth);
		nvgFillColor(vg, color_bg);
		nvgFill(vg);
		nvgFillPaint(vg, bg);
		nvgFill(vg);

		nvgBeginPath(vg);
		nvgRoundedRect(vg, origin.x + strokeWidth * 0.5f, origin.y + strokeWidth * 0.5f, size.x - strokeWidth, size.y - strokeWidth, cornerRadius - strokeWidth * 0.5f);
		nvgStrokeColor(vg, color_stroke);
		nvgStrokeWidth(vg, strokeWidth);
		nvgStroke(vg);

	}

	void RenderButton(NVGcontext* vg, UIButton* btn)
	{
		glm::vec2 origin, size;
		convert_pos(btn->origin_trans, origin);
		convert_size(btn->size, size);
		float cornerRadius = btn->cornerRadius * scale;
		float strokeWidth = btn->strokeWidth * scale;
		NVGcolor color_bg = nvgRGBA(btn->colorBg.r, btn->colorBg.g, btn->colorBg.b, btn->colorBg.a);
		NVGcolor color_stroke = nvgRGBA(btn->colorStroke.r, btn->colorStroke.g, btn->colorStroke.b, btn->colorStroke.a);

		
		if (btn->pressed)
		{
			origin.y += btn->shadowOffset * scale;
		}
		else
		{
			origin.y += btn->shadowOffset * scale;
			nvgBeginPath(vg);
			nvgRoundedRect(vg, origin.x, origin.y, size.x, size.y, cornerRadius);
			nvgFillColor(vg, nvgRGBA(0, 0, 0, 128));
			nvgFill(vg);
			origin.y -= btn->shadowOffset * scale;
		}
		

		NVGpaint bg = nvgLinearGradient(vg, origin.x, origin.y, origin.x, origin.y + size.y, nvgRGBA(255, 255, 255, 32), nvgRGBA(0, 0, 0, 32));
		nvgBeginPath(vg);
		nvgRoundedRect(vg, origin.x + strokeWidth, origin.y + strokeWidth, size.x - strokeWidth * 2.0f, size.y - strokeWidth * 2.0f, cornerRadius - strokeWidth);
		nvgFillColor(vg, color_bg);
		nvgFill(vg);
		nvgFillPaint(vg, bg);
		nvgFill(vg);

		nvgBeginPath(vg);
		nvgRoundedRect(vg, origin.x + strokeWidth * 0.5f, origin.y + strokeWidth * 0.5f, size.x - strokeWidth, size.y - strokeWidth, cornerRadius - strokeWidth * 0.5f);
		nvgStrokeColor(vg, color_stroke);
		nvgStrokeWidth(vg, strokeWidth);
		nvgStroke(vg);
	}

	void RenderScrollViewer(NVGcontext* vg, UIScrollViewer* view)
	{
		glm::vec2 origin, size;
		convert_pos(view->origin_trans, origin);
		convert_size(view->size, size);
		float cornerRadius = view->cornerRadius * scale;
		float strokeWidth = view->strokeWidth * scale;
		NVGcolor color_bg = nvgRGBA(view->colorBg.r, view->colorBg.g, view->colorBg.b, view->colorBg.a);
		NVGcolor color_stroke = nvgRGBA(view->colorStroke.r, view->colorStroke.g, view->colorStroke.b, view->colorStroke.a);

		NVGpaint bg = nvgLinearGradient(vg, origin.x, origin.y, origin.x, origin.y + size.y, nvgRGBA(255, 255, 255, 32), nvgRGBA(0, 0, 0, 32));
		nvgBeginPath(vg);
		nvgRoundedRect(vg, origin.x + strokeWidth, origin.y + strokeWidth, size.x - strokeWidth * 2.0f, size.y - strokeWidth * 2.0f, cornerRadius - strokeWidth);
		nvgFillColor(vg, color_bg);
		nvgFill(vg);
		nvgFillPaint(vg, bg);
		nvgFill(vg);

		nvgBeginPath(vg);
		nvgRoundedRect(vg, origin.x + strokeWidth * 0.5f, origin.y + strokeWidth * 0.5f, size.x - strokeWidth, size.y - strokeWidth, cornerRadius - strokeWidth * 0.5f);
		nvgStrokeColor(vg, color_stroke);
		nvgStrokeWidth(vg, strokeWidth);
		nvgStroke(vg);

	}

	void RenderText(NVGcontext* vg, UIText* ui_text)
	{
		glm::vec2 origin = ui_text->origin_trans;
		if (ui_text->alignment_horizontal == 1 )
		{
			if (ui_text->block == nullptr)
			{
				origin.x += (float)w * 0.5f;
			}
			else
			{
				origin.x += ui_text->block->client_size().x * 0.5f;
			}
		}
		else if (ui_text->alignment_horizontal == 2)
		{
			if (ui_text->block == nullptr)
			{
				origin.x += (float)w;
			}
			else
			{
				origin.x += ui_text->block->client_size().x;
			}
		}

		if (ui_text->alignment_vertical == 1 || ui_text->alignment_vertical == 3)
		{
			if (ui_text->block == nullptr)
			{
				origin.y += (float)h * 0.5f;
			}
			else
			{
				origin.y += ui_text->block->client_size().y * 0.5f;
			}
		}
		else if (ui_text->alignment_vertical == 2)
		{
			if (ui_text->block == nullptr)
			{
				origin.y += (float)h;
			}
			else
			{
				origin.y += ui_text->block->client_size().y;
			}
		}

		convert_pos(origin, origin);

		float font_size = ui_text->font_size * scale;
		int hori_map[3] = { NVG_ALIGN_LEFT, NVG_ALIGN_CENTER, NVG_ALIGN_RIGHT };
		int vert_map[4] = { NVG_ALIGN_TOP, NVG_ALIGN_MIDDLE, NVG_ALIGN_BOTTOM, NVG_ALIGN_BASELINE };
		int align = hori_map[ui_text->alignment_horizontal] | vert_map[ui_text->alignment_vertical];
		NVGcolor color_fg = nvgRGBA(ui_text->colorFg.r, ui_text->colorFg.g, ui_text->colorFg.b, ui_text->colorFg.a);

		nvgFontSize(vg, font_size);
		nvgFontFace(vg, ui_text->font_face.c_str());		
		nvgTextAlign(vg, align);
		nvgFillColor(vg, color_fg);
		nvgText(vg, origin.x, origin.y, ui_text->text.c_str(), nullptr);

	}

	void RenderTextBlock(NVGcontext* vg, UITextBlock* ui_text)
	{
		glm::vec2 origin = ui_text->origin_trans;
		if (ui_text->alignment_horizontal == 1)
		{
			if (ui_text->block == nullptr)
			{
				origin.x += (float)w * 0.5f;
			}
			else
			{
				origin.x += ui_text->block->client_size().x * 0.5f;
			}
		}
		else if (ui_text->alignment_horizontal == 2)
		{
			if (ui_text->block == nullptr)
			{
				origin.x += (float)w;
			}
			else
			{
				origin.x += ui_text->block->client_size().x;
			}
		}

		if (ui_text->alignment_vertical == 1 || ui_text->alignment_vertical == 3)
		{
			if (ui_text->block == nullptr)
			{
				origin.y += (float)h * 0.5f;
			}
			else
			{
				origin.y += ui_text->block->client_size().y * 0.5f;
			}
		}
		else if (ui_text->alignment_vertical == 2)
		{
			if (ui_text->block == nullptr)
			{
				origin.y += (float)h;
			}
			else
			{
				origin.y += ui_text->block->client_size().y;
			}
		}
		convert_pos(origin, origin);

		float line_width = ui_text->line_width * scale;
		float font_size = ui_text->font_size * scale;
		int hori_map[3] = { NVG_ALIGN_LEFT, NVG_ALIGN_CENTER, NVG_ALIGN_RIGHT };
		int vert_map[4] = { NVG_ALIGN_TOP, NVG_ALIGN_MIDDLE, NVG_ALIGN_BOTTOM, NVG_ALIGN_BASELINE };
		int align = hori_map[ui_text->alignment_horizontal] | vert_map[ui_text->alignment_vertical];
		NVGcolor color_fg = nvgRGBA(ui_text->colorFg.r, ui_text->colorFg.g, ui_text->colorFg.b, ui_text->colorFg.a);

		nvgFontSize(vg, font_size);
		nvgFontFace(vg, ui_text->font_face.c_str());
		nvgTextAlign(vg, align);
		nvgTextLineHeight(vg, ui_text->line_height);
		nvgFillColor(vg, color_fg);
		nvgTextBox(vg, origin.x, origin.y, line_width, ui_text->text.c_str(), nullptr);

	}

	void RenderImage(NVGcontext* vg, UIImage* ui_image)
	{
		glm::vec2 origin, size;
		convert_pos(ui_image->origin_trans, origin);
		convert_size(ui_image->size, size);

		NVGpaint pnt = nvgImagePattern(vg, origin.x, origin.y, size.x, size.y, 0.0f, ui_image->id_image, 1.0f);
		nvgBeginPath(vg);
		nvgRect(vg, origin.x, origin.y, size.x, size.y);
		nvgFillPaint(vg, pnt);
		nvgFill(vg);		
	}

	void _get_positions(NVGcontext* vg, UILineEdit* line_edit, std::vector<int>& offsets, std::vector<float>& positions)
	{
		const char* p_str = line_edit->text.c_str();
		float bounds[4];
		float max_pos = nvgTextBounds(vg, 0.0f, 0.0f, p_str, nullptr, bounds);
		NVGglyphPosition glyphs[256];
		int count = nvgTextGlyphPositions(vg, 0.0f, 0.0f, p_str, nullptr, glyphs, 256);
		offsets.resize(count + 1);
		positions.resize(count + 1);
		for (int i = 0; i < count; i++)
		{
			offsets[i] =(int)(glyphs[i].str - p_str);
			positions[i] = glyphs[i].x;
		}
		offsets[count] = (int)line_edit->text.length();
		positions[count] = max_pos;
	}

	void RenderLineEdit(NVGcontext* vg, UILineEdit* line_edit)
	{
		glm::vec2 origin, size;
		convert_pos(line_edit->origin_trans, origin);
		convert_size(line_edit->size, size);

		float font_size = line_edit->font_size * scale;
		NVGcolor color_bg = nvgRGBA(line_edit->colorBg.r, line_edit->colorBg.g, line_edit->colorBg.b, line_edit->colorBg.a);
		NVGcolor color_fg = nvgRGBA(line_edit->colorFg.r, line_edit->colorFg.g, line_edit->colorFg.b, line_edit->colorFg.a);

		float start = 5.0f * scale;
		float end = size.x - 5.0f * scale;

		NVGpaint bg = nvgBoxGradient(vg, origin.x + 1.0f*scale, origin.y + 2.5f * scale, 
			size.x - 2.0f * scale, size.y - 2.0f*scale, 3.0f*scale, 4.0f*scale, color_bg, nvgRGBA(32, 32, 32, 48));

		nvgBeginPath(vg);
		nvgRoundedRect(vg, origin.x + 1.0f*scale, origin.y + 1.0f*scale, size.x - 2.0f*scale, size.y - 2.0f*scale, 3.0f*scale);
		nvgFillPaint(vg, bg);
		nvgFill(vg);

		nvgBeginPath(vg);
		nvgRoundedRect(vg, origin.x + 0.5f*scale, origin.y + 0.5f*scale, size.x - 1.0f*scale, size.y - 1.0f*scale, 3.5f * scale);
		nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 48));
		nvgStroke(vg);

		nvgIntersectScissor(vg, origin.x + start, origin.y + 3.0f * scale, end - start, size.y - 6.0f * scale);
		nvgFontSize(vg, font_size);
		nvgFontFace(vg, line_edit->font_face.c_str());
		nvgFillColor(vg, color_fg);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

		auto& events = line_edit->pendingEvents;
		while (events.size() > 0)
		{
			std::vector<int> offsets;
			std::vector<float> positions;
			_get_positions(vg, line_edit, offsets, positions);
			float p_scroll = positions[line_edit->scroll_pos];

			bool text_changed = false;
			bool cursor_moved = false;

			auto e = events.front();
			events.pop();

			if (e.type == 1)
			{	
				int nearest = 0;
				float prev_pos = -100.0f;
				for (size_t i = 0; i < positions.size(); i++)
				{
					float x = start - p_scroll + positions[i];
					if (x > e.x)
					{
						if (i == 0)
						{
							nearest = 0;							
						}
						else if (e.x - prev_pos > x - e.x)
						{
							nearest = i;
						}
						else
						{
							nearest = i - 1;
						}
						break;
					}
					prev_pos = x;
				}

				line_edit->cursor_pos = nearest;
				cursor_moved = true;
			}
			else if (e.type == 2)
			{
				if (e.code == 37) // left
				{
					if (line_edit->cursor_pos > 0)
					{
						line_edit->cursor_pos--;
						cursor_moved = true;
					}
				}
				else if (e.code == 39) // right
				{
					if (line_edit->cursor_pos < positions.size() - 1)
					{
						line_edit->cursor_pos++;
						cursor_moved = true;
					}
				}
				else if (e.code == 36) // Home
				{
					if (line_edit->cursor_pos != 0)
					{
						line_edit->cursor_pos = 0;
						cursor_moved = true;
					}
				}
				else if (e.code == 35) // End
				{
					if (line_edit->cursor_pos != positions.size() - 1)
					{
						line_edit->cursor_pos = positions.size() - 1;
						cursor_moved = true;
					}
				}
				else if (e.code == 46) // Delete
				{
					if (line_edit->cursor_pos < offsets.size() - 1)
					{
						int start = offsets[line_edit->cursor_pos];
						int end = offsets[line_edit->cursor_pos + 1];
						line_edit->text.erase(start, end - start);
						text_changed = true;
					}
				}
				else if (e.code == 45) // Insert
				{
					line_edit->insert = !line_edit->insert;
				}
				else if (e.code == 8) // Backspace
				{
					if (line_edit->cursor_pos > 0)
					{
						int start = offsets[line_edit->cursor_pos -1];
						int end = offsets[line_edit->cursor_pos];
						line_edit->text.erase(start, end - start);
						text_changed = true;
						line_edit->cursor_pos--;
						cursor_moved = true;
					}
				}
			}
			else if (e.type == 3)
			{
				char text[8];
				cpToUTF8(e.keyChar, text);
				int start = offsets[line_edit->cursor_pos];
				if (!line_edit->insert && line_edit->cursor_pos < offsets.size() - 1)
				{
					int end = offsets[line_edit->cursor_pos + 1];
					line_edit->text.erase(start, end - start);
				}
				line_edit->text.insert(start, text);
				text_changed = true;
				line_edit->cursor_pos++;
				cursor_moved = true;
			}

			if (cursor_moved)
			{
				if (text_changed)
				{
					_get_positions(vg, line_edit, offsets, positions);
				}
				float x = start - p_scroll + positions[line_edit->cursor_pos];
				if (x < start)
				{
					line_edit->scroll_pos = line_edit->cursor_pos;
				}
				else if (x > end)
				{
					float delta = x - end;
					for (size_t i = line_edit->scroll_pos + 1; i < positions.size(); i++)
					{
						if (positions[i] - p_scroll >= delta)
						{
							line_edit->scroll_pos = i;
							break;
						}
					}
				}
			}
		}

		std::vector<int> offsets;
		std::vector<float> positions;
		_get_positions(vg, line_edit, offsets, positions);
		float p_scroll = positions[line_edit->scroll_pos];
		float p_cursor = positions[line_edit->cursor_pos];
		
		nvgText(vg, origin.x + start - p_scroll, origin.y + size.y * 0.5f, line_edit->text.c_str(), nullptr);
		
		if (line_edit->cursor_shown)
		{
			char caret[8];
			if (line_edit->insert)
			{
				cpToUTF8(0x258F, caret);
			}
			else
			{
				cpToUTF8(0x258C, caret);
			}
			nvgText(vg, origin.x + start - p_scroll + p_cursor, origin.y + size.y * 0.5f, caret, nullptr);
		}
		

	}

};

void GLUIRenderer::render(UIManager& UI, int width_wnd, int height_wnd)
{	
	double t = time_sec();
	double delta_t = 0.0;
	if (time_last_render >= 0.0)
	{
		delta_t = t - time_last_render;
	}
	time_last_render = t;

	if (delta_t > 0.0)
	{
		UI.Update(delta_t);
	}

	size_t num_areas = UI.areas.size();	
	for (size_t i = 0; i < num_areas; i++)
	{
		UIArea* area = UI.areas[i];
		AreaRenderer arenderer(area);

		if (area->need_render())
		{	
			arenderer.Render(vg);
			area->appearance_changed = false;
		}

		arenderer.RenderViews(&blitter);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);		
		glEnable(GL_FRAMEBUFFER_SRGB);
		blitter.render(area->render_target.m_tex_video, arenderer.x0, height_wnd - (arenderer.y0 + arenderer.h), arenderer.w, arenderer.h, true);

	}

}

