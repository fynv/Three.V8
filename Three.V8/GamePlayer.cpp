#include <GL/glew.h>
#include <string>
#include <filesystem>
#include <gui/UIManager.h>
#include "GamePlayer.h"

#define ENABLE_MSAA 1

GamePlayer::GamePlayer(const char* exec_path, int width, int height)
	: m_v8vm(exec_path)
#if ENABLE_MSAA
	, m_render_target(true, true)
#else
	, m_render_target(false, false)
#endif
{
	if (width > 0 && height > 0)
	{
		m_render_target.update_framebuffers(width, height);
	}
	m_v8vm.m_isolate->Enter();
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

GamePlayer::~GamePlayer()
{
	UnloadScript();
	m_v8vm.m_isolate->Exit();
}

void GamePlayer::LoadScript(const char* dir, const char* filename)
{
	UnloadScript();
	std::filesystem::current_path(dir);
	m_context = std::unique_ptr<GameContext>(new GameContext(&m_v8vm, this, filename));

	v8::Isolate* isolate = m_v8vm.m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
	v8::Function* callback_init = m_context->GetCallback("init");
	if (callback_init != nullptr)
	{
		std::vector<v8::Local<v8::Value>> args(2);
		args[0] = v8::Number::New(isolate, (double)m_render_target.m_width);
		args[1] = v8::Number::New(isolate, (double)m_render_target.m_height);
		m_context->InvokeCallback(callback_init, args);
	}
}

void GamePlayer::UnloadScript()
{
	if (m_context != nullptr)
	{
		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		v8::Function* callback_dispose = m_context->GetCallback("dispose");
		if (callback_dispose != nullptr)
		{
			m_context->InvokeCallback(callback_dispose, {});
		}
		m_context = nullptr;
	}
}

void GamePlayer::Idle()
{
	if (m_context != nullptr)
	{
		m_context->CheckPendings();		
		v8::platform::PumpMessageLoop(m_v8vm.m_platform.get(), m_v8vm.m_isolate);		
	}
}

void GamePlayer::Draw(int width, int height)
{	
	bool size_changed =  m_render_target.update_framebuffers(width, height);

	if (m_context != nullptr)
	{
		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		v8::Function* callback = m_context->GetCallback("render");
		if (callback != nullptr)
		{
			std::vector<v8::Local<v8::Value>> args(3);
			args[0] = v8::Number::New(isolate, (double)width);
			args[1] = v8::Number::New(isolate, (double)height);
			args[2] = v8::Boolean::New(isolate, size_changed);
			m_context->InvokeCallback(callback, args);
		}
	}

#if !ENABLE_MSAA
	m_render_target.blit_buffer(width, height, 0);
#endif

	if (m_context != nullptr)
	{
		// render UI
		m_ui_renderer.render(*m_context->GetUIManager(), width, height);
	}
}

inline v8::Local<v8::Object> g_CreateMouseEvent(v8::Isolate* isolate, v8::Local<v8::Context> context, int button, int clicks, int delta, int x, int y)
{	
	v8::Local<v8::Object> e = v8::Object::New(isolate);
	e->Set(context, v8::String::NewFromUtf8(isolate, "button").ToLocalChecked(), v8::Number::New(isolate, (double)button));
	e->Set(context, v8::String::NewFromUtf8(isolate, "clicks").ToLocalChecked(), v8::Number::New(isolate, (double)clicks));
	e->Set(context, v8::String::NewFromUtf8(isolate, "delta").ToLocalChecked(), v8::Number::New(isolate, (double)delta));
	e->Set(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), v8::Number::New(isolate, (double)x + 0.5));
	e->Set(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), v8::Number::New(isolate, (double)y + 0.5));
	return e;
}

void GamePlayer::OnMouseDown(int button, int clicks, int delta, int x, int y)
{
	if (m_context != nullptr)
	{
		UIManager* ui_manager = m_context->GetUIManager();
		bool hit = ui_manager->MouseDown(button, clicks, delta, x, y);
		if (hit) return;

		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		v8::Function* callback = m_context->GetCallback("OnMouseDown");
		if (callback != nullptr)
		{						
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(isolate, m_context->m_context.Get(isolate), button, clicks, delta, x, y);
			m_context->InvokeCallback(callback, args);
		}
	}
}

void GamePlayer::OnMouseUp(int button, int clicks, int delta, int x, int y)
{
	if (m_context != nullptr)
	{
		UIManager* ui_manager = m_context->GetUIManager();
		bool hit = ui_manager->MouseUp(button, clicks, delta, x, y);
		if (hit) return;

		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		v8::Function* callback = m_context->GetCallback("OnMouseUp");
		if (callback != nullptr)
		{			
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(isolate, m_context->m_context.Get(isolate), button, clicks, delta, x, y);
			m_context->InvokeCallback(callback, args);
		}
	}
}

void GamePlayer::OnMouseMove(int button, int clicks, int delta, int x, int y)
{
	if (m_context != nullptr)
	{
		UIManager* ui_manager = m_context->GetUIManager();
		bool hit = ui_manager->MouseMove(button, clicks, delta, x, y);
		if (hit) return;

		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		v8::Function* callback = m_context->GetCallback("OnMouseMove");
		if (callback != nullptr)
		{			
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(isolate, m_context->m_context.Get(isolate), button, clicks, delta, x, y);
			m_context->InvokeCallback(callback, args);
		}
	}
}

void GamePlayer::OnMouseWheel(int button, int clicks, int delta, int x, int y)
{
	if (m_context != nullptr)
	{
		UIManager* ui_manager = m_context->GetUIManager();
		bool hit = ui_manager->MouseWheel(button, clicks, delta, x, y);
		if (hit) return;

		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		v8::Function* callback = m_context->GetCallback("OnMouseWheel");
		if (callback != nullptr)
		{			
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(isolate, m_context->m_context.Get(isolate), button, clicks, delta, x, y);
			m_context->InvokeCallback(callback, args);
		}
	}
}


void GamePlayer::OnLongPress(int x, int y)
{
	if (m_context != nullptr)
	{
		UIManager* ui_manager = m_context->GetUIManager();
		bool hit = ui_manager->LongPress((float)x + 0.5f, (float)y + 0.5f);
		if (hit) return;
	}
}

void GamePlayer::OnChar(int keyChar)
{
	if (m_context != nullptr)
	{
		UIManager* ui_manager = m_context->GetUIManager();
		bool handled = ui_manager->KeyChar(keyChar);
		if (handled) return;
	}
}

void GamePlayer::OnControlKey(unsigned code)
{
	if (m_context != nullptr)
	{
		UIManager* ui_manager = m_context->GetUIManager();
		bool handled = ui_manager->ControlKey(code);
		if (handled) return;
	}
}

void GamePlayer::AddMessageHandler(const char* name, MsgHandler handler)
{
	m_msg_map[name] = handler;
}

void GamePlayer::RemoveMessageHandler(const char* name)
{
	auto iter = m_msg_map.find(name);
	if (iter != m_msg_map.end())
	{
		m_msg_map.erase(iter);
	}
}

std::string GamePlayer::UserMessage(const char* name, const char* msg)
{
	auto iter = m_msg_map.find(name);
	if (iter != m_msg_map.end())
	{
		return iter->second.Call(iter->second.ptr, msg);
	}
	return "";
}

std::string GamePlayer::SendMessageToUser(const char* name, const char* msg)
{
	v8::Isolate* isolate = m_v8vm.m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
	v8::Function* callback_init = m_context->GetCallback("message");
	if (callback_init != nullptr)
	{
		std::vector<v8::Local<v8::Value>> args(2);
		args[0] = v8::String::NewFromUtf8(isolate, name).ToLocalChecked();
		args[1] = v8::String::NewFromUtf8(isolate, msg).ToLocalChecked();
		v8::Local<v8::Value> res = m_context->InvokeCallback(callback_init, args).ToLocalChecked();
		v8::String::Utf8Value str(isolate, res);
		return *str;
	}
	return "";
}
