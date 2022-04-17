#pragma once

#include "WrapperUtils.hpp"
#include "lights/Light.hpp"
#include <lights/DirectionalLight.h>


class WrapperDirectionalLight
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetTarget(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetTarget(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};


v8::Local<v8::FunctionTemplate> WrapperDirectionalLight::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperLight::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "target").ToLocalChecked(), GetTarget, SetTarget);
	return templ;
}

void WrapperDirectionalLight::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	DirectionalLight* self = new DirectionalLight();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
}


void WrapperDirectionalLight::GetTarget(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> target = v8::Null(isolate);
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_target").ToLocalChecked()).ToChecked())
	{
		target = holder->Get(context, v8::String::NewFromUtf8(isolate, "_target").ToLocalChecked()).ToLocalChecked();
	}
	info.GetReturnValue().Set(target);
}

void WrapperDirectionalLight::SetTarget(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	DirectionalLight* self = (DirectionalLight*)v8::Local<v8::External>::Cast(holder->GetInternalField(0))->Value();
	holder->Set(context, v8::String::NewFromUtf8(isolate, "_target").ToLocalChecked(), value);
	Object3D* target = (Object3D*)v8::Local<v8::External>::Cast(value.As<v8::Object>()->GetInternalField(0))->Value();
	self->target = target;
}