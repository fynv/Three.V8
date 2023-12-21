#pragma once

#include <string>
#include <unordered_map>
#include <renderers/GLRenderTarget.h>
#include <renderers/GLPickingTarget.h>
#include <renderers/GLUIRenderer.h>
#include "binding.h"
#include "definitions.hpp"

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

	void AddModule(ModuleDefinition&& mod);

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

	void SetPicking(bool picking);
	bool Picking() { return m_pick_target != nullptr; }

	GLPickingTarget* pickingTarget()
	{
		return m_pick_target.get();
	}

private:
	static V8VM& s_get_vm(const char* exec_path);
	V8VM& m_v8vm;

	GLRenderTarget m_render_target;
	std::unique_ptr<GLPickingTarget> m_pick_target;
	GLUIRenderer m_ui_renderer;

	WorldDefinition m_world_definition;
	std::unique_ptr<GameContext> m_context;

	void* m_user_message_callback_data = nullptr;
	UserMessageCallback m_user_message_callback = nullptr;
	
};

