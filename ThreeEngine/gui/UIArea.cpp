#include "UIArea.h"
#include "UIElement.h"

UIArea::UIArea() : render_target(false, false)
{

}

UIArea::~UIArea()
{

}

bool UIArea::need_render()
{
	if (appearance_changed) return true;
	for (size_t i = 0; i < elements.size(); i++)
	{
		if (elements[i]->appearance_changed) return true;
	}
	return false;
}

void UIArea::add(UIElement* object)
{
	this->elements.push_back(object);
	appearance_changed = true;
}

void UIArea::remove(UIElement* object)
{
	auto iter = std::find(elements.begin(), elements.end(), object);
	if (iter != elements.end())
	{
		elements.erase(iter);
		appearance_changed = true;
	}
}

void UIArea::clear()
{
	elements.clear();
	appearance_changed = true;
}
