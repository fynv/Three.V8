#if THREE_MM

#include "WrapperUtils.hpp"
#include <AVCRecorder.h>

#include "WrapperAVCRecorder.h"

void WrapperAVCRecorder::define(ClassDefinition& cls)
{
	cls.name = "AVCRecorder";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "callback",  GetCallback, SetCallback },
	};
}


typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;

struct AVCRecorderCallbackData
{
	GameContext* ctx;
	CallbackT callback;
};


void* WrapperAVCRecorder::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	int id_device = 0;
	if (info.Length() > 0)
	{
		lctx.jnum_to_num(info[0], id_device);
	}

	return new AVCRecorder(id_device);
}

void WrapperAVCRecorder::dtor(void* ptr, GameContext* ctx)
{
	AVCRecorder* self = (AVCRecorder*)ptr;
	{
		AVCRecorderCallbackData* data = (AVCRecorderCallbackData*)self->GetCallbackData();
		if (data != nullptr)
		{
			delete data;
			self->SetCallback(nullptr, nullptr);
		}
	}
	delete self;
}


static void AVCRecorderCallback(const void* msg_data, size_t msg_size, void* ptr)
{
	AVCRecorderCallbackData* data = (AVCRecorderCallbackData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);

	v8::Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, msg_size);
	memcpy(buf->GetBackingStore()->Data(), msg_data, msg_size);

	std::vector<v8::Local<v8::Value>> args = { buf };
	ctx->InvokeCallback(*callback, args);
}


void WrapperAVCRecorder::GetCallback(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> callback = lctx.get_property(lctx.holder, "_callback");
	info.GetReturnValue().Set(callback);
}


void WrapperAVCRecorder::SetCallback(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_callback", value);

	AVCRecorder* self = lctx.self<AVCRecorder>();
	AVCRecorderCallbackData* data = (AVCRecorderCallbackData*)self->GetCallbackData();

	if (data != nullptr)
	{
		delete data;
	}

	data = new AVCRecorderCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->SetCallback(AVCRecorderCallback, data);
}

#endif

