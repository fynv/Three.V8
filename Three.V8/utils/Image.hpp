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
	LocalContext lctx(info);
	Image* self = nullptr;
	if (info.Length() > 1)
	{
		int width, height;
		lctx.jnum_to_num(info[0], width);			
		lctx.jnum_to_num(info[1], height);
		self = new Image(width, height);
	}
	else
	{
		self = new Image();
	}	
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
}

void WrapperImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Image* self = lctx.self<Image>();	
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Image* self = lctx.self<Image>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
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
	LocalContext lctx(info);
	CubeImage* self = new CubeImage();
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}

void WrapperCubeImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeImage* self = lctx.self<CubeImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->images[0].width()));
}

void WrapperCubeImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeImage* self = lctx.self<CubeImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->images[0].height()));
}
