#pragma once

#include <vector>
#include <glm.hpp>

class UIArea;
class UIElement;
class UI3DViewer;

struct HitResult
{
	UIArea* area = nullptr;
	UI3DViewer* first_3dviewer = nullptr;
	UIElement* first_elem = nullptr;
	UIElement* press_elem = nullptr;
	UIElement* scroll_elem = nullptr;
};

class UIManager
{
public:
	std::vector<UIArea*> areas;

	void add(UIArea* object);
	void remove(UIArea* object);
	void clear();

	int pressed_state = 0;
	HitResult hit[3];
	glm::vec2 pos_down;
	glm::vec2 pos_scroll;

	UIElement* focus_elem = nullptr;

	bool MouseDown(int button, int clicks, int delta, int x, int y);
	bool MouseUp(int button, int clicks, int delta, int x, int y);
	bool MouseMove(int button, int clicks, int delta, int x, int y);
	bool MouseWheel(int button, int clicks, int delta, int x, int y);	

	bool TouchDown(int pointerId, float x, float y);
	bool TouchUp(int pointerId, float x, float y);
	bool TouchMove(int pointerId, float x, float y);

	bool LongPress(float x, float y);

	bool KeyChar(int keyChar);
	bool ControlKey(unsigned code);

	void Update(double delta_t);

private:
	HitResult _hit_test(float x, float y);

	bool PointerDown(float x, float y);
	bool PointerUp(float x, float y);
	bool PointerMove(float x, float y);

	void SetFocus(UIElement* elem);

};
