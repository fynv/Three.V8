#pragma once 

#include <queue>
#include <string>
#include "UIElement.h"

class UILineEdit : public UIElement
{
public:
	glm::vec2 size = glm::vec2(100.0f, 40.0f);	
	
	bool insert = true;
	bool cursor_shown = false;
	int cursor_pos = 0;
	int scroll_pos = 0;

	std::string text;

	float font_size = 17.0f;
	std::string font_face = "default";
	glm::u8vec4 colorBg = glm::u8vec4(255, 255, 255, 200);
	glm::u8vec4 colorFg = glm::u8vec4(0, 0, 0, 160);
	
	struct Event
	{
		int type; // 1: click 2: control key 3: char input
		
		// type1
		float x, y;

		// type2
		unsigned code;

		// type3
		int keyChar;
	};

	std::queue<Event> pendingEvents;

};

