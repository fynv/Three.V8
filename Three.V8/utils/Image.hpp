#pragma once

#include "WrapperUtils.hpp"
#include <utils/Image.h>

class WrapperImage
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);	
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperImage::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));	
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
	return templ;
}

void WrapperImage::dtor(void* ptr, GameContext* ctx)
{
	delete (Image*)ptr;
}

void WrapperImage::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	Image* self = nullptr;
	if (info.Length() > 1)
	{
		int width = (int)info[0].As<v8::Number>()->Value();
		int height = (int)info[1].As<v8::Number>()->Value();	
		self = new Image(width, height);
	}
	else
	{
		self = new Image();
	}	
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
}

void WrapperImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Image* self = get_self<Image>(info);
	int width = self == nullptr ? 0 : self->width();
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)width);
	info.GetReturnValue().Set(ret);
}

void WrapperImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Image* self = get_self<Image>(info);
	int height = self == nullptr ? 0 : self->height();
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)height);
	info.GetReturnValue().Set(ret);
}

class WrapperCubeImage
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperCubeImage::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
	return templ;
}


void WrapperCubeImage::dtor(void* ptr, GameContext* ctx)
{
	delete (CubeImage*)ptr;
}

void WrapperCubeImage::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	CubeImage* self = new CubeImage();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
}

void WrapperCubeImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	CubeImage* self = get_self<CubeImage>(info);
	int width = self == nullptr ? 0 : self->images[0].width();
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)width);
	info.GetReturnValue().Set(ret);
}

void WrapperCubeImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	CubeImage* self = get_self<CubeImage>(info);
	int height = self == nullptr ? 0 : self->images[0].height();
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)height);
	info.GetReturnValue().Set(ret);
}
