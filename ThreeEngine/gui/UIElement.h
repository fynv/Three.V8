#pragma once 

#include <glm.hpp>

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

};
