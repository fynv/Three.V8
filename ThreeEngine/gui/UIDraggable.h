#pragma once 

#include "UIPanel.h"

typedef void (*DragCallback)(void* ptr, const glm::vec2& value);

class UIDraggable : public UIPanel
{
public:
	bool draggable_horizontal = true;
	bool draggable_vertical = false;	

	glm::vec2 origin_min = glm::vec2(0.0f, 0.0f);
	glm::vec2 origin_max = glm::vec2(0.0f, 0.0f);

	bool dragging = false;

	void get_value(glm::vec2& value);
	void set_value(const glm::vec2& value);

	DragCallback drag_callback = nullptr;
	void* drag_callback_data = nullptr;


};

