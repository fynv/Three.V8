#pragma once 

#include "UIElement.h"

class UIBlock : public UIElement
{
public:
	glm::vec2 size = glm::vec2(100.0f, 40.0f);

	virtual glm::vec2 client_offset() { return { 0.0f, 0.0f }; }
	virtual glm::vec2 client_size() { return { size.x, size.y }; }

	bool scissor_enabled = true;
	virtual glm::vec2 scissor_offset() { return client_offset(); }
	virtual glm::vec2 scissor_size() { return client_size(); }

};

