#pragma once

#include <glm.hpp>
#include "renderers/GLRenderTarget.h"

typedef void (*RenderCallback)(int width, int height, bool size_changed, void* ptr);
typedef void (*MouseCallback)(int button, int clicks, int delta, int x, int y, void* ptr);
typedef void (*TouchCallback)(int pointerId, float x, float y, void* ptr);

class UIBlock;
class UI3DViewer
{
public:	
	UI3DViewer();
	~UI3DViewer(){}

	UIBlock* block = nullptr;
	glm::vec2 origin = glm::vec2(0.0f, 0.0f);
	glm::vec2 size = glm::vec2(100.0f, 100.0f);

	glm::vec2 origin_trans;
	void update_origin_trans();

	GLRenderTarget render_target;

	RenderCallback render_callback = nullptr;
	void* render_data = nullptr;

	MouseCallback mouse_down_callback = nullptr;
	void* mouse_down_data = nullptr;

	MouseCallback mouse_up_callback = nullptr;
	void* mouse_up_data = nullptr;

	MouseCallback mouse_move_callback = nullptr;
	void* mouse_move_data = nullptr;

	MouseCallback mouse_wheel_callback = nullptr;
	void* mouse_wheel_data = nullptr;

	TouchCallback touch_down_callback = nullptr;
	void* touch_down_data = nullptr;

	TouchCallback touch_up_callback = nullptr;
	void* touch_up_data = nullptr;

	TouchCallback touch_move_callback = nullptr;
	void* touch_move_data = nullptr;
	

};

