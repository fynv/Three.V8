#pragma once

#include <string>
#include <unordered_map>
#include <renderers/GLRenderTarget.h>
#include <renderers/GLUIRenderer.h>
#include "binding.h"

struct MsgHandler
{
	void* ptr = nullptr;
	std::string (*Call)(void* ptr, const char* msg) = nullptr;
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

	typedef std::string(*UserMessageCallback)(void* ptr, const char* name, const char* msg);	
	void SetUserMessageCallback(void* ptr, UserMessageCallback callback);

	std::string UserMessage(const char* name, const char* msg);
	std::string SendMessageToUser(const char* name, const char* msg);	

	GLRenderTarget& renderTarget()
	{
		return m_render_target;
	}

	void SetPrintCallbacks(void* ptr, GameContext::PrintCallback print_callback, GameContext::PrintCallback error_callback);

private:
	static V8VM& s_get_vm(const char* exec_path);
	V8VM& m_v8vm;

	GLRenderTarget m_render_target;
	GLUIRenderer m_ui_renderer;
	
	void* m_print_callback_data = nullptr;
	GameContext::PrintCallback m_print_callback = nullptr;
	GameContext::PrintCallback m_error_callback = nullptr;

	std::unique_ptr<GameContext> m_context;

	void* m_user_message_callback_data = nullptr;
	UserMessageCallback m_user_message_callback = nullptr;
	
};

