#pragma once

#include "WrapperUtils.hpp"
#include <MMCamera.h>

class WrapperMMCamera
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperMMCamera::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));	
	templ->InstanceTemplate()->Set(isolate, "updateTexture", v8::FunctionTemplate::New(isolate, UpdateTexture));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
	return templ;
}

void WrapperMMCamera::dtor(void* ptr, GameContext* ctx)
{
	delete (MMCamera*)ptr;
}

void WrapperMMCamera::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	int idx = 0;
	if (info.Length() > 0)
	{
		lctx.jnum_to_num(info[0], idx);
	}

	MMCamera* self = new MMCamera(idx);
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}

void WrapperMMCamera::UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMCamera* self = lctx.self<MMCamera>();
	self->update_texture();
}


void WrapperMMCamera::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMCamera* self = lctx.self<MMCamera>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperMMCamera::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMCamera* self = lctx.self<MMCamera>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}

