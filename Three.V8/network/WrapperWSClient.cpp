#include "WrapperUtils.hpp"
#include <network/WSClient.h>
#include "WrapperWSClient.h"

void WrapperWSClient::define(ClassDefinition& cls)
{
	cls.name = "WSClient";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "onOpen", GetOnOpen, SetOnOpen },
		{ "onMessage", GetOnMessage, SetOnMessage },
	};
	cls.methods = {
		{"send", Send },		
	};
}


typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;

struct WSClientCallbackData
{
	GameContext* ctx;
	CallbackT callback;
};

void* WrapperWSClient::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string url = lctx.jstr_to_str(info[0]);

	return new WSClient(url.c_str());
}

void WrapperWSClient::dtor(void* ptr, GameContext* ctx)
{
	WSClient* self = (WSClient*)ptr;
	{
		WSClientCallbackData* data = (WSClientCallbackData*)self->GetOpenCallbackData();
		if (data != nullptr)
		{
			delete data;
			self->SetOpenCallback(nullptr, nullptr);
		}
	}
	{
		WSClientCallbackData* data = (WSClientCallbackData*)self->GetMessageCallbackData();
		if (data != nullptr)
		{
			delete data;
			self->SetMessageCallback(nullptr, nullptr);
		}
	}
	delete self;
}

void WrapperWSClient::Send(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	WSClient* self = lctx.self<WSClient>();

	if (info[0]->IsArrayBuffer())
	{
		v8::Local<v8::ArrayBuffer> buf = info[0].As<v8::ArrayBuffer>();
		self->Send(buf->GetBackingStore()->Data(), buf->ByteLength(), true);
	}
	else if (info[0]->IsString())
	{
		std::string str = lctx.jstr_to_str(info[0]);
		self->Send(str.c_str(), str.length(), false);
	}
}

static void WSClientOpenCallback(void* ptr)
{
	WSClientCallbackData* data = (WSClientCallbackData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);
	std::vector<v8::Local<v8::Value>> args;
	ctx->InvokeCallback(*callback, args);

}


void WrapperWSClient::GetOnOpen(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onOpen = lctx.get_property(lctx.holder, "_onOpen");
	info.GetReturnValue().Set(onOpen);
}

void WrapperWSClient::SetOnOpen(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onOpen", value);

	WSClient* self = lctx.self<WSClient>();
	WSClientCallbackData* data = (WSClientCallbackData*)self->GetOpenCallbackData();

	if (data != nullptr)
	{
		delete data;
	}

	data = new WSClientCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->SetOpenCallback(WSClientOpenCallback, data);
}


static void WSClientMessageCallback(const void* msg_data, size_t msg_size, bool is_binary, void* ptr)
{
	WSClientCallbackData* data = (WSClientCallbackData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);

	if (is_binary)
	{
		v8::Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, msg_size);
		memcpy(buf->GetBackingStore()->Data(), msg_data, msg_size);
		std::vector<v8::Local<v8::Value>> args = { buf };
		ctx->InvokeCallback(*callback, args);

	}
	else
	{
		v8::Local<v8::String> str = v8::String::NewFromUtf8(isolate, (const char*)msg_data, v8::NewStringType::kNormal, (int)msg_size).ToLocalChecked();
		std::vector<v8::Local<v8::Value>> args = { str };
		ctx->InvokeCallback(*callback, args);
	}
}


void WrapperWSClient::GetOnMessage(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onMessage = lctx.get_property(lctx.holder, "_onMessage");
	info.GetReturnValue().Set(onMessage);
}


void WrapperWSClient::SetOnMessage(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onMessage", value);

	WSClient* self = lctx.self<WSClient>();
	WSClientCallbackData* data = (WSClientCallbackData*)self->GetMessageCallbackData();

	if (data != nullptr)
	{
		delete data;
	}

	data = new WSClientCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->SetMessageCallback(WSClientMessageCallback, data);
}

