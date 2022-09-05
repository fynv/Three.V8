#pragma once

#include <renderers/GLRenderTarget.h>
#include <renderers/GLUIRenderer.h>
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
	GamePlayer(const char* exec_path, int width, int height);
	~GamePlayer();	

	int width() const { return m_render_target.m_width; }
	int height() const { return m_render_target.m_height; }

	GLUIRenderer& UIRenderer() { return m_ui_renderer; }

	void Idle();
	void Draw(int width, int height);
	void LoadScript(const char* dir, const char* filename);
	void UnloadScript();

	void OnMouseDown(int button, int clicks, int delta, int x, int y);	
	void OnMouseUp(int button, int clicks, int delta, int x, int y);
	void OnMouseMove(int button, int clicks, int delta, int x, int y);
	void OnMouseWheel(int button, int clicks, int delta, int x, int y);	
	void OnLongPress(int x, int y);

	void OnChar(int keyChar);
	void OnControlKey(unsigned code);

	// window calls
	void SetWindowCalls(const WindowCalls& windowCalls)
	{
		m_windowCalls = windowCalls;
	}
	void SetMouseCapture();
	void ReleaseMouseCapture();

	GLRenderTarget& renderTarget()
	{
		return m_render_target;
	}

private:
	V8VM m_v8vm;

	GLRenderTarget m_render_target;
	GLUIRenderer m_ui_renderer;
	
	std::unique_ptr<GameContext> m_context;

	WindowCalls m_windowCalls;
	
};

