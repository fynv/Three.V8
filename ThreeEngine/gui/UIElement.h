#pragma once 

#include <glm.hpp>

typedef void (*PointCallback)(float x, float y, void* ptr);

class UIBlock;
class UIElement
{
public:
	UIElement() {  }
	virtual ~UIElement() {}
	UIBlock* block = nullptr;
	glm::vec2 origin = glm::vec2(0.0f, 0.0f);

	bool appearance_changed = true;

	// used during rendering;
	glm::vec2 origin_trans;
	void update_origin_trans();

	PointCallback pointer_down_callback = nullptr;
	void* pointer_down_callback_data = nullptr;

	PointCallback pointer_up_callback = nullptr;
	void* pointer_up_callback_data = nullptr;

	PointCallback pointer_move_callback = nullptr;
	void* pointer_move_callback_data = nullptr;

};
