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
};


v8::Local<v8::ObjectTemplate> WrapperGamePlayer::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->SetInternalFieldCount(1);
	templ->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
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
