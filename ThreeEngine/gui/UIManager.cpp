#include "UIManager.h"
#include "UIArea.h"
#include "UIElement.h"
#include "UIBlock.h"
#include "UIButton.h" 
#include "UIImage.h"
#include "UIScrollViewer.h"
#include "UI3DViewer.h"
#include "UILineEdit.h"
#include "UIDraggable.h"
#include "utils/Utils.h"

void UIManager::add(UIArea* object)
{
	this->areas.push_back(object);	
}

void UIManager::remove(UIArea* object)
{
	auto iter = std::find(areas.begin(), areas.end(), object);
	if (iter != areas.end())
	{		
		areas.erase(iter);
	}	
}

void UIManager::clear()
{	
	areas.clear();
}

class AreaIntersector
{
public:
	UIArea* area;
	float scale, x0, y0, x1, y1;

	AreaIntersector(UIArea* area) : area(area)
	{
		scale = area->scale;
		x0 = area->origin.x * scale;
		y0 = area->origin.y * scale;
		x1 = x0 + area->size.x * scale;
		y1 = y0 + area->size.y * scale;
	}

	HitResult hit(float x, float y)
	{
		HitResult ret;
		if (x<x0 || x>x1 || y<y0 || y>y1) return ret;

		size_t num_viewers = area->viewers.size();
		for (size_t i = num_viewers - 1; i != (size_t)(-1); i--)
		{
			UI3DViewer* viewer = area->viewers[i];

			bool clipped = false;
			UIBlock* block = viewer->block;
			while (block != nullptr)
			{
				if (block->scissor_enabled)
				{
					glm::vec2 origin, size;
					origin.x = block->origin_trans.x * scale + x0;
					origin.y = block->origin_trans.y * scale + y0;
					size.x = block->size.x * scale;
					size.y = block->size.y * scale;

					glm::vec2 scissor_origin = origin + block->scissor_offset() * scale;
					glm::vec2 scissor_size = block->scissor_size() * scale;

					if (x < scissor_origin.x || y < scissor_origin.y)
					{
						clipped = true;
						break;
					}

					if (x > scissor_origin.x + scissor_size.x || y > scissor_origin.y + scissor_size.y)
					{
						clipped = true;
						break;
					}
				}				
				block = block->block;
			}
			if (clipped) continue;

			{
				glm::vec2 origin, size;
				origin.x = viewer->origin_trans.x * scale + x0;
				origin.y = viewer->origin_trans.y * scale + y0;
				size.x = viewer->size.x * scale;
				size.y = viewer->size.y * scale;
				if (x < origin.x || y < origin.y) continue;
				if (x > origin.x + size.x || y > origin.y + size.y) continue;
				ret.area = area;
				ret.first_3dviewer = viewer;
				break;
			}			
		}
		if (ret.first_3dviewer != nullptr) return ret;

		size_t num_elements = area->elements.size();
		for (size_t i = num_elements - 1; i != (size_t)(-1); i--)
		{
			UIElement* elem = area->elements[i];
			
			bool clipped = false;
			UIBlock* block = elem->block;
			while (block != nullptr)
			{
				UIScrollViewer* view = dynamic_cast<UIScrollViewer*>(block);
				if (view != nullptr)
				{
					glm::vec2 origin, size;
					origin.x = view->origin_trans.x * scale + x0;
					origin.y = view->origin_trans.y * scale + y0;
					size.x = view->size.x * scale;
					size.y = view->size.y * scale;

					glm::vec2 scissor_origin = origin + view->scissor_offset() * scale;
					glm::vec2 scissor_size = view->scissor_size() * scale;

					if (x < scissor_origin.x || y < scissor_origin.y)
					{
						clipped = true;
						break;
					}

					if (x > scissor_origin.x + scissor_size.x || y > scissor_origin.y + scissor_size.y)
					{
						clipped = true;
						break;
					}
				}
				block = block->block;
			}
			if (clipped) continue;

			do
			{
				UIBlock* block = dynamic_cast<UIBlock*>(elem);
				if (block != nullptr)
				{
					glm::vec2 origin, size;
					origin.x = block->origin_trans.x * scale + x0;
					origin.y = block->origin_trans.y * scale + y0;
					size.x = block->size.x * scale;
					size.y = block->size.y * scale;
					if (x < origin.x || y < origin.y) break;
					if (x > origin.x + size.x || y > origin.y + size.y) break;
					ret.first_elem = block;
					break;
				}

				UILineEdit* edit = dynamic_cast<UILineEdit*>(elem);
				if (edit != nullptr)
				{
					glm::vec2 origin, size;
					origin.x = edit->origin_trans.x * scale + x0;
					origin.y = edit->origin_trans.y * scale + y0;
					size.x = edit->size.x * scale;
					size.y = edit->size.y * scale;
					if (x < origin.x || y < origin.y) break;
					if (x > origin.x + size.x || y > origin.y + size.y) break;
					ret.first_elem = edit;
					break;
				}

				UIImage* ui_image = dynamic_cast<UIImage*>(elem);
				if (ui_image != nullptr)
				{
					glm::vec2 origin, size;
					origin.x = ui_image->origin_trans.x * scale + x0;
					origin.y = ui_image->origin_trans.y * scale + y0;
					size.x = ui_image->size.x * scale;
					size.y = ui_image->size.y * scale;
					if (x < origin.x || y < origin.y) break;
					if (x > origin.x + size.x || y > origin.y + size.y) break;
					ret.first_elem = elem;
					break;
				}
			}
			while (false);

			if (ret.first_elem != nullptr) break;
		}

		if (ret.first_elem != nullptr)
		{
			ret.area = area;
			UIElement* elem = ret.first_elem;
			do
			{
				if (ret.press_elem == nullptr)
				{
					do
					{
						UIButton* btn = dynamic_cast<UIButton*>(elem);
						if (btn != nullptr)
						{
							ret.press_elem = btn;
							break;
						}

						UIDraggable* draggable = dynamic_cast<UIDraggable*>(elem);
						if (draggable != nullptr)
						{
							ret.press_elem = draggable;
							break;
						}

					} while (false);
				}
				if (ret.scroll_elem == nullptr)
				{
					UIScrollViewer* view = dynamic_cast<UIScrollViewer*>(elem);
					if (view != nullptr)
					{
						ret.scroll_elem = view;
					}
				}
				elem = elem->block;
			} while (elem != nullptr && (ret.press_elem==nullptr || ret.scroll_elem==nullptr));

		}

		return ret;

	}
};

void UIManager::SetFocus(UIElement* elem)
{
	if (elem != nullptr)
	{
		do
		{
			UILineEdit* edit = dynamic_cast<UILineEdit*>(elem);
			if (edit != nullptr) break;
			elem = nullptr;
		} while (false);
	}

	if (elem == nullptr && focus_elem != nullptr)
	{
		UILineEdit* edit = dynamic_cast<UILineEdit*>(focus_elem);
		if (edit != nullptr)
		{
			if (edit->cursor_shown)
			{
				edit->cursor_shown = false;
				edit->appearance_changed = true;
			}
		}
	}

	focus_elem = elem;
}

bool UIManager::MouseDown(int button, int clicks, int delta, int x, int y)
{
	hit[0] = _hit_test((float)x + 0.5f, (float)y + 0.5f);
	if (hit[0].first_3dviewer != nullptr)
	{
		UI3DViewer* viewer = hit[0].first_3dviewer;
		glm::vec2 origin, size;
		origin.x = (viewer->origin_trans.x + hit[0].area->origin.x) * hit[0].area->scale;
		origin.y = (viewer->origin_trans.y + hit[0].area->origin.y) * hit[0].area->scale;

		int vx = (int)((float)x + 0.5f - origin.x);
		int vy = (int)((float)y + 0.5f - origin.y);

		
		if (viewer->mouse_down_callback != nullptr)
		{
			viewer->mouse_down_callback(button, clicks, delta, vx, vy, viewer->mouse_down_data);
		}

		SetFocus(nullptr);

		return true;
	}

	if (button == 0)
	{
		return PointerDown((float)x + 0.5f, (float)y + 0.5f);
	}
	return false;
}

bool UIManager::MouseUp(int button, int clicks, int delta, int x, int y)
{
	if (hit[0].first_3dviewer != nullptr)
	{
		UI3DViewer* viewer = hit[0].first_3dviewer;
		glm::vec2 origin, size;
		origin.x = (viewer->origin_trans.x + hit[0].area->origin.x) * hit[0].area->scale;
		origin.y = (viewer->origin_trans.y + hit[0].area->origin.y) * hit[0].area->scale;

		int vx = (int)((float)x + 0.5f - origin.x);
		int vy = (int)((float)y + 0.5f - origin.y);

		if (viewer->mouse_up_callback != nullptr)
		{
			viewer->mouse_up_callback(button, clicks, delta, vx, vy, viewer->mouse_up_data);
		}

		hit[0].first_3dviewer = nullptr;

		return true;
	}

	if (button == 0)
	{
		return PointerUp((float)x + 0.5f, (float)y + 0.5f);
	}
	return false;
}

bool UIManager::MouseMove(int button, int clicks, int delta, int x, int y)
{
	if (hit[0].first_3dviewer != nullptr)
	{
		UI3DViewer* viewer = hit[0].first_3dviewer;
		glm::vec2 origin, size;
		origin.x = (viewer->origin_trans.x + hit[0].area->origin.x) * hit[0].area->scale;
		origin.y = (viewer->origin_trans.y + hit[0].area->origin.y) * hit[0].area->scale;

		int vx = (int)((float)x + 0.5f - origin.x);
		int vy = (int)((float)y + 0.5f - origin.y);

		if (viewer->mouse_move_callback != nullptr)
		{
			viewer->mouse_move_callback(button, clicks, delta, vx, vy, viewer->mouse_move_data);
		}

		return true;
	}

	if (button == 0)
	{
		return PointerMove((float)x + 0.5f, (float)y + 0.5f);
	}
	return false;
}

bool UIManager::MouseWheel(int button, int clicks, int delta, int x, int y)
{
	if (hit[0].first_elem == nullptr && hit[0].first_3dviewer == nullptr)
	{
		hit[0] = _hit_test(x, y);

		if (hit[0].first_3dviewer != nullptr)
		{
			UI3DViewer* viewer = hit[0].first_3dviewer;
			glm::vec2 origin, size;
			origin.x = (viewer->origin_trans.x + hit[0].area->origin.x) * hit[0].area->scale;
			origin.y = (viewer->origin_trans.y + hit[0].area->origin.y) * hit[0].area->scale;

			int vx = (int)((float)x + 0.5f - origin.x);
			int vy = (int)((float)y + 0.5f - origin.y);

			if (viewer->mouse_wheel_callback != nullptr)
			{
				viewer->mouse_wheel_callback(button, clicks, delta, vx, vy, viewer->mouse_wheel_data);
			}

			hit[0].first_3dviewer = nullptr;
			return true;
		}

		if (hit[0].first_elem != nullptr)
		{
			hit[0].first_elem = nullptr;
			return true;
		}
	}
	return false;
}


bool UIManager::TouchDown(int pointerId, float x, float y)
{
	if (pointerId > 2) return false;

	hit[pointerId] = _hit_test(x, y);

	if (hit[pointerId].first_3dviewer != nullptr)
	{
		UI3DViewer* viewer = hit[pointerId].first_3dviewer;
		glm::vec2 origin, size;
		origin.x = (viewer->origin_trans.x + hit[pointerId].area->origin.x) * hit[pointerId].area->scale;
		origin.y = (viewer->origin_trans.y + hit[pointerId].area->origin.y) * hit[pointerId].area->scale;

		float vx = x - origin.x;
		float vy = y - origin.y;

		if (viewer->touch_down_callback != nullptr)
		{
			viewer->touch_down_callback(pointerId, vx, vy, viewer->touch_down_data);
		}

		SetFocus(nullptr);

		return true;
	}

	if (pointerId == 0)
	{
		return PointerDown(x, y);
	}
	return false;
}

bool UIManager::TouchUp(int pointerId, float x, float y)
{
	if (pointerId > 2) return false;
	if (hit[pointerId].first_3dviewer != nullptr)
	{
		UI3DViewer* viewer = hit[pointerId].first_3dviewer;
		glm::vec2 origin, size;
		origin.x = (viewer->origin_trans.x + hit[pointerId].area->origin.x) * hit[pointerId].area->scale;
		origin.y = (viewer->origin_trans.y + hit[pointerId].area->origin.y) * hit[pointerId].area->scale;

		float vx = x - origin.x;
		float vy = y - origin.y;

		if (viewer->touch_up_callback != nullptr)
		{
			viewer->touch_up_callback(pointerId, vx, vy, viewer->touch_up_data);
		}

		hit[pointerId].first_3dviewer = nullptr;

		return true;
	}

	if (pointerId == 0)
	{
		return PointerUp(x, y);
	}
	return false;
}

bool UIManager::TouchMove(int pointerId, float x, float y)
{
	if (pointerId > 2) return false;
	if (hit[pointerId].first_3dviewer != nullptr)
	{
		UI3DViewer* viewer = hit[pointerId].first_3dviewer;
		glm::vec2 origin, size;
		origin.x = (viewer->origin_trans.x + hit[pointerId].area->origin.x) * hit[pointerId].area->scale;
		origin.y = (viewer->origin_trans.y + hit[pointerId].area->origin.y) * hit[pointerId].area->scale;

		float vx = x - origin.x;
		float vy = y - origin.y;

		if (viewer->touch_move_callback != nullptr)
		{
			viewer->touch_move_callback(pointerId, vx, vy, viewer->touch_move_data);
		}

		return true;
	}

	if (pointerId == 0)
	{
		return PointerMove(x, y);
	}
	return false;
}

HitResult UIManager::_hit_test(float x, float y)
{
	HitResult res;
	size_t num_areas = areas.size();
	for (size_t i = num_areas - 1; i != (size_t)(-1); i--)
	{
		UIArea* area = areas[i];
		AreaIntersector inter(area);
		res = inter.hit(x, y);
		if (res.first_3dviewer!=nullptr || res.first_elem != nullptr) break;
	}
	return res;
}

bool UIManager::PointerDown(float x, float y)
{
	if (hit[0].press_elem != nullptr)
	{
		do
		{
			UIButton* btn = dynamic_cast<UIButton*>(hit[0].press_elem);
			if (btn != nullptr)
			{
				btn->pressed = true;
				btn->appearance_changed = true;
				break;
			}

			UIDraggable* draggable = dynamic_cast<UIDraggable*>(hit[0].press_elem);
			if (draggable != nullptr)
			{
				draggable->dragging = true;			
				pos_record = draggable->origin;
				break;
			}
		} while (false);
		pressed_state = 1;
	}

	pos_down.x = x;
	pos_down.y = y;
	if (hit[0].scroll_elem != nullptr)
	{
		UIScrollViewer* view = dynamic_cast<UIScrollViewer*>(hit[0].scroll_elem);
		if (view != nullptr)
		{
			view->dragging = true;
			pos_scroll = view->scroll_position;
		}
	}

	SetFocus(hit[0].first_elem);

	if (hit[0].first_elem != nullptr)
	{
		UIElement* elem = hit[0].first_elem;
		UIArea* area = hit[0].area;
		float scale = area->scale;
		float x0 = area->origin.x * scale;
		float y0 = area->origin.y * scale;
		glm::vec2 origin;
		origin.x = elem->origin_trans.x * scale + x0;
		origin.y = elem->origin_trans.y * scale + y0;

		UILineEdit* edit = dynamic_cast<UILineEdit*>(hit[0].first_elem);
		if (edit != nullptr)
		{
			UILineEdit::Event e;
			e.type = 1;
			e.x = x - origin.x;
			e.y = y - origin.y;
			edit->pendingEvents.push(e);
			edit->appearance_changed = true;
		}

		if (elem->pointer_down_callback != nullptr)
		{
			elem->pointer_down_callback(x, y, elem->pointer_down_callback_data);
		}
		return true;
	}

	return false;
	
}

bool UIManager::PointerUp(float x, float y)
{	
	if (hit[0].press_elem!=nullptr)
	{
		if (pressed_state == 1)
		{
			do
			{
				UIButton* btn = dynamic_cast<UIButton*>(hit[0].press_elem);
				if (btn != nullptr)
				{
					if (btn->pressed)
					{
						if (btn->click_callback != nullptr)
						{
							btn->click_callback(btn->click_callback_data);
						}
						btn->pressed = false;
						btn->appearance_changed = true;
					}
					break;
				}

				UIDraggable* draggable = dynamic_cast<UIDraggable*>(hit[0].press_elem);
				if (draggable != nullptr)
				{
					draggable->dragging = false;
					break;
				}

			} while (false);
		}
		pressed_state = 0;
	}	

	if (hit[0].scroll_elem != nullptr)
	{
		UIScrollViewer* view = dynamic_cast<UIScrollViewer*>(hit[0].scroll_elem);
		if (view != nullptr)
		{
			view->dragging = false;
			view->appearance_changed = true;
		}
	}
	
	if (hit[0].first_elem != nullptr)
	{
		UIElement* elem = hit[0].first_elem;
		if (elem->pointer_up_callback != nullptr)
		{
			elem->pointer_up_callback(x, y, elem->pointer_up_callback_data);
		}
		hit[0].first_elem = nullptr;
		return true;
	}
	return false;
}

bool UIManager::PointerMove(float x, float y)
{
	glm::vec2 delta;
	if (hit[0].area != nullptr)
	{
		delta = (glm::vec2(x, y) - pos_down) / hit[0].area->scale;
	} 

	if (hit[0].scroll_elem != nullptr)
	{
		if (hit[0].press_elem != nullptr && pressed_state == 1)
		{			
			float dis = glm::length(delta);
			if (dis > 3.0f)
			{
				do
				{
					UIButton* btn = dynamic_cast<UIButton*>(hit[0].press_elem);
					if (btn != nullptr && btn->pressed)
					{
						btn->pressed = false;
						btn->appearance_changed = true;
						break;
					}

					UIDraggable* draggable = dynamic_cast<UIDraggable*>(hit[0].press_elem);
					if (draggable != nullptr)
					{
						draggable->dragging = false;
						break;
					}
				} while (false);
				hit[0].press_elem = nullptr;
				pressed_state = 0;
			}
		}

		UIScrollViewer* view = dynamic_cast<UIScrollViewer*>(hit[0].scroll_elem);
		if (view != nullptr)
		{
			glm::vec2 scroll_bound = view->content_size - view->scissor_size();
			if (scroll_bound.x < 0.0f) scroll_bound.x = 0.0f;
			if (scroll_bound.y < 0.0f) scroll_bound.y = 0.0f;

			if (view->scrollable_horizontal)
			{				
				view->scroll_position.x = pos_scroll.x - delta.x;
				if (view->scroll_position.x < 0.0f) view->scroll_position.x = 0.0f;
				if (view->scroll_position.x > scroll_bound.x) view->scroll_position.x = scroll_bound.x;
				view->appearance_changed = true;
			}
			
			if (view->scrollable_vertical)
			{
				view->scroll_position.y = pos_scroll.y - delta.y;
				if (view->scroll_position.y < 0.0f) view->scroll_position.y = 0.0f;
				if (view->scroll_position.y > scroll_bound.y) view->scroll_position.y = scroll_bound.y;
				view->appearance_changed = true;
			}			
		}
	}

	if (hit[0].press_elem != nullptr && pressed_state == 1)
	{		
		do
		{
			UIButton* btn = dynamic_cast<UIButton*>(hit[0].press_elem);
			if (btn != nullptr)
			{
				AreaIntersector inter(hit[0].area);
				HitResult res = inter.hit(x, y);
				if (res.press_elem == hit[0].press_elem)
				{
					if (!btn->pressed)
					{
						btn->pressed = true;
						btn->appearance_changed = true;
					}
				}
				else
				{
					if (btn->pressed)
					{
						btn->pressed = false;
						btn->appearance_changed = true;
					}
				}
				break;
			}

			UIDraggable* draggable = dynamic_cast<UIDraggable*>(hit[0].press_elem);
			if (draggable != nullptr)
			{
				if (draggable->draggable_horizontal)
				{
					draggable->origin.x = pos_record.x + delta.x;
					if (draggable->origin.x < draggable->origin_min.x) draggable->origin.x = draggable->origin_min.x;
					if (draggable->origin.x > draggable->origin_max.x) draggable->origin.x = draggable->origin_max.x;
					draggable->appearance_changed = true;
				}

				if (draggable->draggable_vertical)
				{
					draggable->origin.y += pos_record.y + delta.y;
					if (draggable->origin.y < draggable->origin_min.y) draggable->origin.y = draggable->origin_min.y;
					if (draggable->origin.y > draggable->origin_max.y) draggable->origin.y = draggable->origin_max.y;
					draggable->appearance_changed = true;
				}
				
				glm::vec2 value;
				draggable->get_value(value);

				if (draggable->drag_callback != nullptr)
				{
					draggable->drag_callback(draggable->drag_callback_data, value);
				}
				break;
			}
		} while (false);
	}


	if (hit[0].first_elem != nullptr)
	{
		UIElement* elem = hit[0].first_elem;
		if (elem->pointer_move_callback != nullptr)
		{
			elem->pointer_move_callback(x, y, elem->pointer_move_callback_data);
		}		
		return true;
	}
	return false;
}

bool UIManager::LongPress(float x, float y)	
{
	if (hit[0].press_elem != nullptr && pressed_state == 1)
	{		
		UIButton* btn = dynamic_cast<UIButton*>(hit[0].press_elem);
		if (btn != nullptr)
		{
			pressed_state = 2;
			if (btn->pressed)
			{
				if (btn->long_press_callback != nullptr)
				{
					btn->long_press_callback(btn->long_press_callback_data);
				}
				btn->pressed = false;
				btn->appearance_changed = true;
			}

			hit[0].press_elem = nullptr;
		}
	}
	return hit[0].first_elem != nullptr;
}

bool UIManager::KeyChar(int keyChar)
{
	if (focus_elem != nullptr)
	{
		UILineEdit* edit = dynamic_cast<UILineEdit*>(focus_elem);
		if (edit != nullptr)
		{
			UILineEdit::Event e;
			e.type = 3;
			e.keyChar = keyChar;
			edit->pendingEvents.push(e);
			edit->appearance_changed = true;
		}
		return true;
	}
	return false;
}


bool UIManager::ControlKey(unsigned code)
{
	if (focus_elem != nullptr)
	{
		UILineEdit* edit = dynamic_cast<UILineEdit*>(focus_elem);
		if (edit != nullptr)
		{
			UILineEdit::Event e;
			e.type = 2;
			e.code = code;
			edit->pendingEvents.push(e);
			edit->appearance_changed = true;
		}
		return true;
	}
	return false;
}

class AreaUpdator
{
public:
	UIArea* area;
	double delta_t;
	float scale;

	AreaUpdator(UIArea* area, double delta_t) 
		: area(area)
		, delta_t(delta_t)
	{
		scale = area->scale;
	}

	void Update()
	{	
		size_t num_elements = area->elements.size();
		for (size_t i = 0; i < num_elements; i++)
		{
			UIElement* elem = area->elements[i];
			UIScrollViewer* view = dynamic_cast<UIScrollViewer*>(elem);
			if (view != nullptr)
			{
				UpdateScrollViewer(view);
			}
		}
	}

	void UpdateScrollViewer(UIScrollViewer* view)
	{
		if (view->dragging)
		{
			glm::vec2 v = (view->scroll_position - view->last_scroll_pos) * (float)(1.0 / delta_t);
			view->velocity = 0.4f * view->velocity + 0.6f * v;
		}
		else if (glm::length(view->velocity) > 0.0f)
		{
			glm::vec2 scroll_bound = view->content_size - view->scissor_size();
			if (scroll_bound.x < 0.0f) scroll_bound.x = 0.0f;
			if (scroll_bound.y < 0.0f) scroll_bound.y = 0.0f;

			view->scroll_position += view->velocity * (float)delta_t;
			if (view->scroll_position.x < 0.0f) view->scroll_position.x = 0.0f;
			if (view->scroll_position.x > scroll_bound.x) view->scroll_position.x = scroll_bound.x;
			if (view->scroll_position.y < 0.0f) view->scroll_position.y = 0.0f;
			if (view->scroll_position.y > scroll_bound.y) view->scroll_position.y = scroll_bound.y;

			view->appearance_changed = true;

			glm::vec2 dir = glm::normalize(view->velocity);
			glm::vec2 acc = dir * (500.0f / scale) * (float)delta_t;
			if (glm::length(acc) > glm::length(view->velocity))
			{
				view->velocity = { 0.0f, 0.0f };
			}
			else
			{
				view->velocity -= acc;
			}

		}
		view->last_scroll_pos = view->scroll_position;
	}
};

void UIManager::Update(double delta_t)
{
	size_t num_areas = areas.size();
	for (size_t i = 0; i < num_areas; i++)
	{
		UIArea* area = areas[i];
		AreaUpdator updator(area, delta_t);
		updator.Update();
	}

	if (focus_elem != nullptr)
	{
		UILineEdit* edit = dynamic_cast<UILineEdit*>(focus_elem);
		if (edit != nullptr)
		{
			double t = time_sec();
			double frag = t - floor(t);
			bool show = frag < 0.5;
			if (edit->cursor_shown != show)
			{
				edit->cursor_shown = show;
				edit->appearance_changed = true;
			}

		}
	}
}