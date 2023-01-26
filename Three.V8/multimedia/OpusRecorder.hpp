#pragma once

#include "WrapperUtils.hpp"
#include <OpusRecorder.h>

class WrapperOpusRecorder
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);

	static void GetCallback(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetCallback(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};


v8::Local<v8::FunctionTemplate> WrapperOpusRecorder::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "callback").ToLocalChecked(), GetCallback, SetCallback);

	return templ;
}

typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;

struct OpusRecorderCallbackData
{
	GameContext* ctx;
	CallbackT callback;
};


void WrapperOpusRecorder::dtor(void* ptr, GameContext* ctx)
{
	OpusRecorder* self = (OpusRecorder*)ptr;
	{
		OpusRecorderCallbackData* data = (OpusRecorderCallbackData*)self->GetCallbackData();
		if (data != nullptr)
		{
			delete data;
		}
	}	
	ctx->remove_opus_recorder(self);
	delete self;
}


void WrapperOpusRecorder::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	int id_device = -1;
	if (info.Length() > 0)
	{
		lctx.jnum_to_num(info[0], id_device);
	}

	OpusRecorder* self = new OpusRecorder(id_device);
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
	lctx.ctx()->add_opus_recorder(self);
}


static void OpusRecorderCallback(const void* msg_data, size_t msg_size, void* ptr)
{
	OpusRecorderCallbackData* data = (OpusRecorderCallbackData*)ptr;
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


void WrapperOpusRecorder::GetCallback(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> callback = lctx.get_property(lctx.holder, "_callback");
	info.GetReturnValue().Set(callback);
}


void WrapperOpusRecorder::SetCallback(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_callback", value);

	OpusRecorder* self = lctx.self<OpusRecorder>();
	OpusRecorderCallbackData* data = (OpusRecorderCallbackData*)self->GetCallbackData();

	if (data != nullptr)
	{
		delete data;
	}

	data = new OpusRecorderCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->SetCallback(OpusRecorderCallback, data);
}