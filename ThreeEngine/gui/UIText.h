#pragma once 

#include <string>
#include "UIElement.h"

class UIText : public UIElement
{
public:
	std::string text;

	float font_size = 17.0f;
	std::string font_face = "default";
	int alignment_horizontal = 1; // 0,1,2
	int alignment_vertical = 1; // 0,1,2
	glm::u8vec4 colorFg = glm::u8vec4(0,0,0,160);


};

