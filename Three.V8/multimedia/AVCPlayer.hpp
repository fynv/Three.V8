#pragma once

#include "WrapperUtils.hpp"
#include <AVCPlayer.h>

class WrapperAVCPlayer
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void AddPacket(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperAVCPlayer::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "addPacket", v8::FunctionTemplate::New(isolate, AddPacket));
	templ->InstanceTemplate()->Set(isolate, "updateTexture", v8::FunctionTemplate::New(isolate, UpdateTexture));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
	return templ;
}


void WrapperAVCPlayer::dtor(void* ptr, GameContext* ctx)
{
	delete (AVCPlayer*)ptr;
}

void WrapperAVCPlayer::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AVCPlayer* self = new AVCPlayer;
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}


void WrapperAVCPlayer::AddPacket(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AVCPlayer* self = lctx.self<AVCPlayer>();

	v8::Local<v8::ArrayBuffer> arr = info[0].As<v8::ArrayBuffer>();
	size_t size = arr->ByteLength();
	const uint8_t* data = (const uint8_t*)arr->GetBackingStore()->Data();
	self->AddPacket(size, data);

}

void WrapperAVCPlayer::UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AVCPlayer* self = lctx.self<AVCPlayer>();
	self->update_texture();
}

void WrapperAVCPlayer::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AVCPlayer* self = lctx.self<AVCPlayer>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperAVCPlayer::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AVCPlayer* self = lctx.self<AVCPlayer>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}

