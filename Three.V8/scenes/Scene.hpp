#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <scenes/Scene.h>
#include <backgrounds/Background.h>
#include <lights/IndirectLight.h>
#include <scenes/Fog.h>

class WrapperScene
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetBackground(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetBackground(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetIndirectLight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetIndirectLight(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetFog(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetFog(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};

v8::Local<v8::FunctionTemplate> WrapperScene::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "background").ToLocalChecked(), GetBackground, SetBackground);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "indirectLight").ToLocalChecked(), GetIndirectLight, SetIndirectLight);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "fog").ToLocalChecked(), GetFog, SetFog);
	return templ;
}

void WrapperScene::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	Scene* self = new Scene();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), WrapperObject3D::dtor);
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
	Scene* self = (Scene*)holder->GetAlignedPointerFromInternalField(0);
	holder->Set(context, v8::String::NewFromUtf8(isolate, "_background").ToLocalChecked(), value);
	if (!value->IsNull())
	{
		Background* background = (Background*)value.As<v8::Object>()->GetAlignedPointerFromInternalField(0);
		self->background = background;
	}
	else
	{
		self->background = nullptr;
	}
}


void WrapperScene::GetIndirectLight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> environmentMap = v8::Null(isolate);
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_indirectLight").ToLocalChecked()).ToChecked())
	{
		environmentMap = holder->Get(context, v8::String::NewFromUtf8(isolate, "_indirectLight").ToLocalChecked()).ToLocalChecked();
	}
	info.GetReturnValue().Set(environmentMap);
}

void WrapperScene::SetIndirectLight(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	Scene* self = (Scene*)holder->GetAlignedPointerFromInternalField(0);
	holder->Set(context, v8::String::NewFromUtf8(isolate, "_indirectLight").ToLocalChecked(), value);
	if (!value->IsNull())
	{
		IndirectLight* indirectLight = (IndirectLight*)value.As<v8::Object>()->GetAlignedPointerFromInternalField(0);
		self->indirectLight = indirectLight;
	}
	else
	{
		self->indirectLight = nullptr;
	}
}


void WrapperScene::GetFog(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> environmentMap = v8::Null(isolate);
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_fog").ToLocalChecked()).ToChecked())
	{
		environmentMap = holder->Get(context, v8::String::NewFromUtf8(isolate, "_fog").ToLocalChecked()).ToLocalChecked();
	}
	info.GetReturnValue().Set(environmentMap);
}

void WrapperScene::SetFog(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	Scene* self = (Scene*)holder->GetAlignedPointerFromInternalField(0);
	holder->Set(context, v8::String::NewFromUtf8(isolate, "_fog").ToLocalChecked(), value);
	if (!value->IsNull())
	{
		Fog* fog = (Fog*)value.As<v8::Object>()->GetAlignedPointerFromInternalField(0);
		self->fog = fog;
	}
	else
	{
		self->fog = nullptr;
	}
}

