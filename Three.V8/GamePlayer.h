#pragma once

#include "binding.h"

struct WindowCalls
{
	void* window = nullptr;
	void (*SetMouseCapture)(void* window) = nullptr;
	void (*ReleaseMouseCapture)(void* window) = nullptr;
};

class GamePlayer
{
public:
	GamePlayer(V8VM* v8vm, int width, int height);
	~GamePlayer();

	int width() const { return m_width; }
	int height() const { return m_height; }

	void Draw(int width, int height);
	void LoadScript(const char* dir, const char* filename);

	void OnMouseDown(int button, int clicks, int delta, int x, int y);
	void OnMouseUp(int button, int clicks, int delta, int x, int y);
	void OnMouseMove(int button, int clicks, int delta, int x, int y);
	void OnMouseWheel(int button, int clicks, int delta, int x, int y);	

	// window calls
	void SetWindowCalls(const WindowCalls& windowCalls)
	{
		m_windowCalls = windowCalls;
	}
	void SetMouseCapture();
	void ReleaseMouseCapture();

private:
	int m_width = -1;
	int m_height = -1;

	V8VM* m_v8vm;
	std::unique_ptr<GameContext> m_context;

	void _unloadScript();

	WindowCalls m_windowCalls;
	
};

