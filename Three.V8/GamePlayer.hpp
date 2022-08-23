#pragma once

#include "WrapperUtils.hpp"
#include "GamePlayer.h"

class WrapperGamePlayer
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	
	static void SetMouseCapture(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void ReleaseMouseCapture(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void HasFont(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreateFontFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreateFontFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::ObjectTemplate> WrapperGamePlayer::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->SetInternalFieldCount(1);
	templ->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
	templ->Set(isolate, "setMouseCapture", v8::FunctionTemplate::New(isolate, SetMouseCapture));
	templ->Set(isolate, "releaseMouseCapture", v8::FunctionTemplate::New(isolate, ReleaseMouseCapture));
	templ->Set(isolate, "hasFont", v8::FunctionTemplate::New(isolate, HasFont));
	templ->Set(isolate, "createFontFromFile", v8::FunctionTemplate::New(isolate, CreateFontFromFile));
	templ->Set(isolate, "createFontFromMemory", v8::FunctionTemplate::New(isolate, CreateFontFromMemory));
	return templ;
}

void WrapperGamePlayer::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	GamePlayer* self = get_self<GamePlayer>(info);
	int width = self->width();
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)width));
}

void WrapperGamePlayer::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	GamePlayer* self = get_self<GamePlayer>(info);
	int height = self->height();
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)height));
}

void WrapperGamePlayer::SetMouseCapture(const v8::FunctionCallbackInfo<v8::Value>& info)
{	
	GamePlayer* self = get_self<GamePlayer>(info);
	self->SetMouseCapture();
}

void WrapperGamePlayer::ReleaseMouseCapture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	GamePlayer* self = get_self<GamePlayer>(info);
	self->ReleaseMouseCapture();
}

void WrapperGamePlayer::HasFont(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	GamePlayer* self = get_self<GamePlayer>(info);
	v8::String::Utf8Value name(info.GetIsolate(), info[0]);
	bool has_font = self->UIRenderer().HasFont(*name);
	info.GetReturnValue().Set(v8::Boolean::New(info.GetIsolate(), has_font));
}

void WrapperGamePlayer::CreateFontFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	GamePlayer* self = get_self<GamePlayer>(info);
	v8::String::Utf8Value name(info.GetIsolate(), info[0]);
	v8::String::Utf8Value filename(info.GetIsolate(), info[1]);
	self->UIRenderer().CreateFont(*name, *filename);
}

void WrapperGamePlayer::CreateFontFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	GamePlayer* self = get_self<GamePlayer>(info);
	v8::String::Utf8Value name(info.GetIsolate(), info[0]);
	v8::Local<v8::ArrayBuffer> data = info[1].As<v8::ArrayBuffer>();
	self->UIRenderer().CreateFont(*name, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());
}

