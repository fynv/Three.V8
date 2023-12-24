#pragma once 

#include <glm.hpp>
#include <vector>
#include "renderers/UIAreaRenderTarget.h"

class UIElement;
class UI3DViewer;
class UIArea
{
public:
	UIArea();
	~UIArea();
	std::vector<UIElement*> elements;
	std::vector<UI3DViewer*> viewers;

	float scale = 1.0f;
	glm::vec2 origin = glm::vec2(0.0f, 0.0f);
	glm::vec2 size = glm::vec2(0.0f, 0.0f);	

	UIAreaRenderTarget render_target;
	bool appearance_changed = true;
	bool need_render();	

	void add(UIElement* object);
	void remove(UIElement* object);
	void clear();

};

