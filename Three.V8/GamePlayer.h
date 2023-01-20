#pragma once

#include <string>
#include <unordered_map>
#include <renderers/GLRenderTarget.h>
#include <renderers/GLUIRenderer.h>
#include "binding.h"

struct MsgHandler
{
	void* window = nullptr;
	std::string (*Call)(void* window, const char* msg) = nullptr;
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

	void AddMessageHandler(const char* name, MsgHandler handler);
	void RemoveMessageHandler(const char* name);

	std::string UserMessage(const char* name, const char* msg);
	std::string SendMessageToUser(const char* name, const char* msg);	

	GLRenderTarget& renderTarget()
	{
		return m_render_target;
	}

private:
	V8VM m_v8vm;

	GLRenderTarget m_render_target;
	GLUIRenderer m_ui_renderer;
	
	std::unique_ptr<GameContext> m_context;
	
	std::unordered_map<std::string, MsgHandler> m_msg_map;	
};

