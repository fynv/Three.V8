#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <scenes/Scene.h>
#include <backgrounds/Background.h>

class WrapperScene
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetBackground(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetBackground(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};

v8::Local<v8::FunctionTemplate> WrapperScene::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "background").ToLocalChecked(), GetBackground, SetBackground);
	return templ;
}

void WrapperScene::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	Scene* self = new Scene();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
	WrapperObject3D::Init(info.GetIsolate(), info.This());
}

void WrapperScene::GetBackground(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> background = v8::Null(isolate);
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_background").ToLocalChecked()).ToChecked())
	{
		background = holder->Get(context, v8::String::NewFromUtf8(isolate, "_background").ToLocalChecked()).ToLocalChecked();
	}
	info.GetReturnValue().Set(background);
}

void WrapperScene::SetBackground(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	Scene* self = (Scene*)v8::Local<v8::External>::Cast(holder->GetInternalField(0))->Value();
	holder->Set(context, v8::String::NewFromUtf8(isolate, "_background").ToLocalChecked(), value);
	Background* background = (Background*)v8::Local<v8::External>::Cast(value.As<v8::Object>()->GetInternalField(0))->Value();
	self->background = background;
}