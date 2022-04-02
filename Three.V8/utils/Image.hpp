#pragma once

#include "WrapperUtils.hpp"
#include <utils/Image.h>

class WrapperImage
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void Dispose(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetHasAlpha(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperImage::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(1);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, Dispose));
	templ->InstanceTemplate()->Set(isolate, "loadFile", v8::FunctionTemplate::New(isolate, LoadFile));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "hasAlpha").ToLocalChecked(), GetHasAlpha, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
	return templ;
}

void WrapperImage::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), nullptr));
}


void WrapperImage::Dispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	Image* self = get_self<Image>(info);
	delete self;
}

void WrapperImage::LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::String::Utf8Value filename(isolate, info[0]);
	Image* self = get_self<Image>(info);
	delete self;
	self = new Image(*filename);
	info.Holder()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
}

void WrapperImage::GetHasAlpha(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Image* self = get_self<Image>(info);
	bool has_alpha = self == nullptr ? false : self->has_alpha();
	v8::Local<v8::Boolean> ret = v8::Boolean::New(info.GetIsolate(), has_alpha);
	info.GetReturnValue().Set(ret);
}

void WrapperImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Image* self = get_self<Image>(info);
	int width = self == nullptr ? false : self->width();
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)width);
	info.GetReturnValue().Set(ret);
}

void WrapperImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Image* self = get_self<Image>(info);
	int height = self == nullptr ? false : self->height();
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)height);
	info.GetReturnValue().Set(ret);
}
