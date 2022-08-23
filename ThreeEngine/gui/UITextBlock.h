#pragma once 

#include <string>
#include "UIElement.h"


class UITextBlock : public UIElement
{
public:
	std::string text;
	float line_width = 150.0f;
	float line_height = 1.2f;

	float font_size = 17.0f;
	std::string font_face = "default";
	int alignment_horizontal = 0; // 0,1,2
	int alignment_vertical = 0; // 0,1,2
	glm::u8vec4 colorFg = glm::u8vec4(0, 0, 0, 160);


};

