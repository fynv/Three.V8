#pragma once 

#include "UIBlock.h"

class UIScrollViewer : public UIBlock
{
public:
	float cornerRadius = 5.0f;
	float strokeWidth = 2.0f;
	glm::u8vec4 colorBg = glm::u8vec4(160, 160, 160, 255);
	glm::u8vec4 colorStroke = glm::u8vec4(0, 0, 0, 255);

	glm::vec2 scissor_offset() override;
	glm::vec2 scissor_size() override;

	bool scrollable_vertical = true;
	bool scrollable_horizontal = false;
	glm::vec2 scroll_position = glm::vec2(0.0f, 0.0f);
	glm::vec2 content_size = glm::vec2(100.0f, 100.0f);

	glm::vec2 client_offset() override;
	glm::vec2 client_size() override;

	bool dragging = false;
	glm::vec2 last_scroll_pos;
	glm::vec2 velocity = glm::vec2(0.0f, 0.0f);

	
};

