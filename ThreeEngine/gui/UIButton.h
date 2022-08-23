#pragma once 

#include "UIBlock.h"

typedef void (*ClickCallback)(void* ptr);

class UIButton : public UIBlock
{
public:
	float cornerRadius = 5.0f;
	float strokeWidth = 1.0f;
	float shadowOffset = 3.0f;
	bool pressed = false;
	glm::u8vec4 colorBg = glm::u8vec4(206, 147, 216, 255);
	glm::u8vec4 colorStroke = glm::u8vec4(0, 0, 0, 255);


	glm::vec2 client_offset() override;
	glm::vec2 client_size() override;

	ClickCallback click_callback = nullptr;
	void* click_callback_data = nullptr;

	ClickCallback long_press_callback = nullptr;
	void* long_press_callback_data = nullptr;

};

