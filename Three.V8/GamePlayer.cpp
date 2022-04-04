#include <GL/glew.h>
#include <string>
#include <filesystem>
#include "GamePlayer.h"

bool GamePlayer::_update_framebuffers(int width, int height)
{
	bool size_changed = m_width != width || m_height != height;
	if (size_changed)
	{
		if (m_fbo_msaa == -1)
		{
			glGenFramebuffers(1, &m_fbo_msaa);
			glGenTextures(1, &m_tex_msaa);
			glGenRenderbuffers(1, &m_rbo_msaa);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_SRGB8_ALPHA8, width, height, true);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo_msaa);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo_msaa);

		m_width = width;
		m_height = height;
	}
	return size_changed;
}

GamePlayer::GamePlayer(V8VM* v8vm, int width, int height) : m_v8vm(v8vm)
{
	_update_framebuffers(width, height);
}

GamePlayer::~GamePlayer()
{
	_unloadScript();

	if (m_fbo_msaa != -1)
		glDeleteFramebuffers(1, &m_fbo_msaa);
	if (m_tex_msaa != -1)
		glDeleteTextures(1, &m_tex_msaa);
	if (m_rbo_msaa != -1)
		glDeleteRenderbuffers(1, &m_rbo_msaa);
}

void GamePlayer::LoadScript(const char* dir, const char* filename)
{
	_unloadScript();
	std::filesystem::current_path(dir);
	m_context = std::unique_ptr<GameContext>(new GameContext(m_v8vm, this, filename));

	v8::Context::Scope context_scope(m_context->m_context.Get(m_v8vm->m_isolate));
	v8::Function* callback_init = m_context->GetCallback("init");
	if (callback_init != nullptr)
	{
		v8::Isolate* isolate = m_context->m_vm->m_isolate;
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
		v8::Context::Scope context_scope(m_context->m_context.Get(m_v8vm->m_isolate));
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
	GLint backbufId = 0;

	bool size_changed = _update_framebuffers(width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);


	if (m_context != nullptr)
	{
		v8::Context::Scope context_scope(m_context->m_context.Get(m_v8vm->m_isolate));
		v8::Function* callback = m_context->GetCallback("render");
		if (callback != nullptr)
		{
			v8::Isolate* isolate = m_context->m_vm->m_isolate;
			std::vector<v8::Local<v8::Value>> args(3);
			args[0] = v8::Number::New(isolate, (double)width);
			args[1] = v8::Number::New(isolate, (double)height);
			args[2] = v8::Boolean::New(isolate, size_changed);
			m_context->InvokeCallback(callback, args);
		}
	}	

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, backbufId);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, backbufId);
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
		v8::Context::Scope context_scope(m_context->m_context.Get(m_v8vm->m_isolate));
		v8::Function* callback = m_context->GetCallback("OnMouseDown");
		if (callback != nullptr)
		{			
			v8::Isolate* isolate = m_context->m_vm->m_isolate;
			v8::HandleScope handle_scope(isolate);			
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(isolate, m_context->m_context.Get(m_v8vm->m_isolate), button, clicks, delta, x, y);
			m_context->InvokeCallback(callback, args);
		}
	}
}

void GamePlayer::OnMouseUp(int button, int clicks, int delta, int x, int y)
{
	if (m_context != nullptr)
	{
		v8::Context::Scope context_scope(m_context->m_context.Get(m_v8vm->m_isolate));
		v8::Function* callback = m_context->GetCallback("OnMouseUp");
		if (callback != nullptr)
		{
			v8::Isolate* isolate = m_context->m_vm->m_isolate;
			v8::HandleScope handle_scope(isolate);
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(isolate, m_context->m_context.Get(m_v8vm->m_isolate), button, clicks, delta, x, y);
			m_context->InvokeCallback(callback, args);
		}
	}
}

void GamePlayer::OnMouseMove(int button, int clicks, int delta, int x, int y)
{
	if (m_context != nullptr)
	{
		v8::Context::Scope context_scope(m_context->m_context.Get(m_v8vm->m_isolate));
		v8::Function* callback = m_context->GetCallback("OnMouseMove");
		if (callback != nullptr)
		{
			v8::Isolate* isolate = m_context->m_vm->m_isolate;
			v8::HandleScope handle_scope(isolate);
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(isolate, m_context->m_context.Get(m_v8vm->m_isolate), button, clicks, delta, x, y);
			m_context->InvokeCallback(callback, args);
		}
	}
}

void GamePlayer::OnMouseWheel(int button, int clicks, int delta, int x, int y)
{
	if (m_context != nullptr)
	{
		v8::Context::Scope context_scope(m_context->m_context.Get(m_v8vm->m_isolate));
		v8::Function* callback = m_context->GetCallback("OnMouseWheel");
		if (callback != nullptr)
		{
			v8::Isolate* isolate = m_context->m_vm->m_isolate;
			v8::HandleScope handle_scope(isolate);
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(isolate, m_context->m_context.Get(m_v8vm->m_isolate), button, clicks, delta, x, y);
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