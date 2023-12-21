#include <GL/glew.h>
#include <string>
#include <filesystem>
#include <gui/UIManager.h>
#include <utils/AsyncCallbacks.h>
#include "GamePlayer.h"
#include "DefaultModule.h"
#include "WrapperUtils.hpp"
#include "WrapperGamePlayer.h"

#define ENABLE_MSAA 1

V8VM& GamePlayer::s_get_vm(const char* exec_path)
{
	static thread_local V8VM vm(exec_path);
	return vm;
}

GamePlayer::GamePlayer(const char* exec_path, int width, int height)
	: m_v8vm(s_get_vm(exec_path))
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
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	srand(time(nullptr));

	WrapperGamePlayer::s_game_player = this;
	GetDefaultModule(m_world_definition.default_module);
}

GamePlayer::~GamePlayer()
{
	UnloadScript();
}

void GamePlayer::AddModule(ModuleDefinition&& mod)
{
	m_world_definition.modules.emplace_back(mod);
}

void GamePlayer::LoadScript(const char* dir, const char* filename)
{
	UnloadScript();
	std::filesystem::current_path(dir);
	m_context = std::unique_ptr<GameContext>(new GameContext(&m_v8vm, m_world_definition, filename));	

	v8::Isolate* isolate = m_v8vm.m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
	LocalContext lctx(isolate);
	v8::Function* callback_init = m_context->GetCallback("init");
	if (callback_init != nullptr)
	{
		std::vector<v8::Local<v8::Value>> args(2);
		args[0] = lctx.num_to_jnum(m_render_target.m_width);
		args[1] = lctx.num_to_jnum(m_render_target.m_height);
		m_context->InvokeCallback(callback_init, args);
	}
}

void GamePlayer::UnloadScript()
{
	AsyncCallbacks::CheckPendings();
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
	AsyncCallbacks::CheckPendings();
	v8::platform::PumpMessageLoop(m_v8vm.m_platform.get(), m_v8vm.m_isolate);
}

void GamePlayer::Draw(int width, int height)
{	
	int cur_fbo;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &cur_fbo);
	m_render_target.m_fbo_default = cur_fbo;

	bool size_changed =  m_render_target.update_framebuffers(width, height);
	if (size_changed && m_pick_target != nullptr)
	{
		m_pick_target->update_framebuffers(width, height);
	}

	if (m_context != nullptr)
	{
		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		LocalContext lctx(isolate);

		v8::Function* callback = m_context->GetCallback("render");
		if (callback != nullptr)
		{
			std::vector<v8::Local<v8::Value>> args(3);
			args[0] = lctx.num_to_jnum(width);
			args[1] = lctx.num_to_jnum(height);
			args[2] = v8::Boolean::New(isolate, size_changed);
			m_context->InvokeCallback(callback, args);
		}
#if !ENABLE_MSAA
		m_render_target.blit_buffer(width, height, 0);
#endif		
		v8::Local<v8::Value> value = lctx.get_global("UIManager");
		UIManager* ui_manager = lctx.jobj_to_obj<UIManager>(value);

		// render UI
		m_ui_renderer.render(*ui_manager, width, height, cur_fbo);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, cur_fbo);
}

inline v8::Local<v8::Object> g_CreateMouseEvent(LocalContext* lctx, int button, int clicks, int delta, int x, int y)
{	
	v8::Local<v8::Object> e = v8::Object::New(lctx->isolate);
	lctx->set_property(e, "button", lctx->num_to_jnum(button));
	lctx->set_property(e, "clicks", lctx->num_to_jnum(clicks));
	lctx->set_property(e, "delta", lctx->num_to_jnum(delta));
	lctx->set_property(e, "x", lctx->num_to_jnum((double)x + 0.5));
	lctx->set_property(e, "y", lctx->num_to_jnum((double)y + 0.5));
	return e;
}

void GamePlayer::OnMouseDown(int button, int clicks, int delta, int x, int y)
{
	if (m_context != nullptr)
	{
		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		LocalContext lctx(isolate);
		v8::Local<v8::Value> value = lctx.get_global("UIManager");
		UIManager* ui_manager = lctx.jobj_to_obj<UIManager>(value);
		bool hit = ui_manager->MouseDown(button, clicks, delta, x, y);
		if (hit) return;		
		
		v8::Function* callback = m_context->GetCallback("OnMouseDown");
		if (callback != nullptr)
		{						
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(&lctx, button, clicks, delta, x, y);
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
		LocalContext lctx(isolate);
		v8::Local<v8::Value> value = lctx.get_global("UIManager");
		UIManager* ui_manager = lctx.jobj_to_obj<UIManager>(value);
		bool hit = ui_manager->MouseUp(button, clicks, delta, x, y);
		if (hit) return;
		
		v8::Function* callback = m_context->GetCallback("OnMouseUp");
		if (callback != nullptr)
		{			
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(&lctx, button, clicks, delta, x, y);
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
		LocalContext lctx(isolate);
		v8::Local<v8::Value> value = lctx.get_global("UIManager");
		UIManager* ui_manager = lctx.jobj_to_obj<UIManager>(value);
		bool hit = ui_manager->MouseMove(button, clicks, delta, x, y);
		if (hit) return;
		
		v8::Function* callback = m_context->GetCallback("OnMouseMove");
		if (callback != nullptr)
		{			
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(&lctx, button, clicks, delta, x, y);
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
		LocalContext lctx(isolate);
		v8::Local<v8::Value> value = lctx.get_global("UIManager");
		UIManager* ui_manager = lctx.jobj_to_obj<UIManager>(value);
		bool hit = ui_manager->MouseWheel(button, clicks, delta, x, y);
		if (hit) return;
		
		v8::Function* callback = m_context->GetCallback("OnMouseWheel");
		if (callback != nullptr)
		{			
			std::vector<v8::Local<v8::Value>> args(1);
			args[0] = g_CreateMouseEvent(&lctx, button, clicks, delta, x, y);
			m_context->InvokeCallback(callback, args);
		}
	}
}


void GamePlayer::OnLongPress(int x, int y)
{
	if (m_context != nullptr)
	{
		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		LocalContext lctx(isolate);
		v8::Local<v8::Value> value = lctx.get_global("UIManager");
		UIManager* ui_manager = lctx.jobj_to_obj<UIManager>(value);
		bool hit = ui_manager->LongPress((float)x + 0.5f, (float)y + 0.5f);
		if (hit) return;
	}
}

void GamePlayer::OnChar(int keyChar)
{
	if (m_context != nullptr)
	{
		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		LocalContext lctx(isolate);
		v8::Local<v8::Value> value = lctx.get_global("UIManager");
		UIManager* ui_manager = lctx.jobj_to_obj<UIManager>(value);
		bool handled = ui_manager->KeyChar(keyChar);
		if (handled) return;
	}
}

void GamePlayer::OnControlKey(unsigned code)
{
	if (m_context != nullptr)
	{
		v8::Isolate* isolate = m_v8vm.m_isolate;
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(m_context->m_context.Get(isolate));
		LocalContext lctx(isolate);
		v8::Local<v8::Value> value = lctx.get_global("UIManager");
		UIManager* ui_manager = lctx.jobj_to_obj<UIManager>(value);
		bool handled = ui_manager->ControlKey(code);
		if (handled) return;
	}
}

void GamePlayer::SetUserMessageCallback(void* ptr, UserMessageCallback callback)
{
	m_user_message_callback_data = ptr;
	m_user_message_callback = callback;
}

std::string GamePlayer::UserMessage(const char* name, const char* msg)
{
	if (m_user_message_callback != nullptr)
	{
		return m_user_message_callback(m_user_message_callback_data, name, msg);
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
		v8::Local<v8::Value> res;
		if (m_context->InvokeCallback(callback_init, args).ToLocal(&res))
		{
			v8::String::Utf8Value str(isolate, res);
			return *str;
		}
	}
	return "";
}

void GamePlayer::SetPicking(bool picking)
{
	if (picking)
	{
		m_pick_target = std::unique_ptr<GLPickingTarget>(new GLPickingTarget);
		m_pick_target->update_framebuffers(m_render_target.m_width, m_render_target.m_height);
	}
	else
	{
		m_pick_target = nullptr;
	}

}
