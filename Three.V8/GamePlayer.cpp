#include <GL/glew.h>
#include <string>
#include <filesystem>
#include "GamePlayer.h"

#define ENABLE_MSAA 1

GamePlayer::GamePlayer(const char* exec_path, int width, int height)
	: m_v8vm(exec_path)
	, m_width(width)
	, m_height(height)
#if ENABLE_MSAA
	, m_render_target(true, true)
#else
	, m_render_target(false, false)
#endif
{
	m_v8vm.m_isolate->Enter();
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

GamePlayer::~GamePlayer()
{
	_unloadScript();
	m_v8vm.m_isolate->Exit();
}

void GamePlayer::LoadScript(const char* dir, const char* filename)
{
	_unloadScript();
	std::filesystem::current_path(dir);
	m_context = std::unique_ptr<GameContext>(new GameContext(&m_v8vm, this, filename));

	v8::Isolate* isolate = m_v8vm.m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
	v8::Function* callback_init = m_context->GetCallback("init");
	if (callback_init != nullptr)
	{
		std::vector<v8::Local<v8::Value>> args(2);
		args[0] = v8::Number::New(isolate, (double)m_width);
		args[1] = v8::Number::New(isolate, (double)m_height);
		m_context->InvokeCallback(callback_init, args);
	}
}

void GamePlayer::_unloadScript()
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


void GamePlayer::Draw(int width, int height)
{
	bool size_changed = m_width != width || m_height != height;
	if (size_changed)
	{
		m_width = width;
		m_height = height;
	}
	m_render_target.update_framebuffers(width, height);

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

#if ENABLE_MSAA
	m_render_target.resolve_msaa();
#else
	m_render_target.blit_buffer(width, height, 0);
#endif

}

inline v8::Local<v8::Object> g_CreateMouseEvent(v8::Isolate* isolate, v8::Local<v8::Context> context, int button, int clicks, int delta, int x, int y)
{	
	v8::Local<v8::Object> e = v8::Object::New(isolate);
	e->Set(context, v8::String::NewFromUtf8(isolate, "button").ToLocalChecked(), v8::Number::New(isolate, (double)button));
	e->Set(context, v8::String::NewFromUtf8(isolate, "clicks").ToLocalChecked(), v8::Number::New(isolate, (double)clicks));
	e->Set(context, v8::String::NewFromUtf8(isolate, "delta").ToLocalChecked(), v8::Number::New(isolate, (double)delta));
	e->Set(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), v8::Number::New(isolate, (double)x));
	e->Set(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), v8::Number::New(isolate, (double)y));
	return e;
}

void GamePlayer::OnMouseDown(int button, int clicks, int delta, int x, int y)
{
	if (m_context != nullptr)
	{
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

void GamePlayer::SetMouseCapture()
{
	if (m_windowCalls.window != nullptr && m_windowCalls.SetMouseCapture != nullptr)
	{
		m_windowCalls.SetMouseCapture(m_windowCalls.window);
	}
}

void GamePlayer::ReleaseMouseCapture()
{
	if (m_windowCalls.window != nullptr && m_windowCalls.ReleaseMouseCapture != nullptr)
	{
		m_windowCalls.ReleaseMouseCapture(m_windowCalls.window);
	}
}