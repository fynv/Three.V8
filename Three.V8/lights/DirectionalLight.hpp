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
	static void SetShadow(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetShadowProjection(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperDirectionalLight::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperLight::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "target").ToLocalChecked(), GetTarget, SetTarget);
	templ->InstanceTemplate()->Set(isolate, "setShadow", v8::FunctionTemplate::New(isolate, SetShadow));
	templ->InstanceTemplate()->Set(isolate, "setShadowProjection", v8::FunctionTemplate::New(isolate, SetShadowProjection));
	return templ;
}

void WrapperDirectionalLight::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	DirectionalLight* self = new DirectionalLight();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
	info.This()->SetInternalField(1, v8::External::New(info.GetIsolate(), WrapperObject3D::dtor));
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This());
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

void WrapperDirectionalLight::SetShadow(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	DirectionalLight* self = get_self<DirectionalLight>(info);

	bool enable = info[0].As<v8::Boolean>()->Value();
	int width = -1;
	int height = -1;
	if (info.Length() > 2)
	{
		width = (int)info[1].As<v8::Number>()->Value();
		height = (int)info[2].As<v8::Number>()->Value();
	}
	self->setShadow(enable, width, height);
}

void  WrapperDirectionalLight::SetShadowProjection(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	DirectionalLight* self = get_self<DirectionalLight>(info);

	float left = (float)info[0].As<v8::Number>()->Value();
	float right = (float)info[1].As<v8::Number>()->Value();
	float bottom = (float)info[2].As<v8::Number>()->Value();
	float top = (float)info[3].As<v8::Number>()->Value();
	float zNear = (float)info[4].As<v8::Number>()->Value();
	float zFar = (float)info[5].As<v8::Number>()->Value();
	self->setShadowProjection(left, right, bottom, top, zNear, zFar);
}

