#pragma once 

#include "UIBlock.h"

class UIPanel : public UIBlock
{
public:
	float cornerRadius = 5.0f;
	float strokeWidth = 2.0f;
	glm::u8vec4 colorBg = glm::u8vec4(160, 160, 160, 255);
	glm::u8vec4 colorStroke = glm::u8vec4(0, 0, 0, 255);


	glm::vec2 client_offset() override;
	glm::vec2 client_size() override;

};

