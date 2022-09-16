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
	LocalContext lctx(info);
	Scene* self = new Scene();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), WrapperObject3D::dtor);
}

void WrapperScene::GetBackground(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> background = lctx.get_property(info.Holder(), "_background");
	info.GetReturnValue().Set(background);
}

void WrapperScene::SetBackground(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();
	if (!value->IsNull())
	{
		Background* background = lctx.jobj_to_obj<Background>(value);
		self->background = background;
	}
	else
	{
		self->background = nullptr;
	}
	lctx.set_property(info.Holder(), "_background", value);
	
}


void WrapperScene::GetIndirectLight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> indirectLight = lctx.get_property(info.Holder(), "_indirectLight");
	info.GetReturnValue().Set(indirectLight);
}

void WrapperScene::SetIndirectLight(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();
	if (!value->IsNull())
	{
		IndirectLight* indirectLight = lctx.jobj_to_obj<IndirectLight>(value);
		self->indirectLight = indirectLight;
	}
	else
	{
		self->indirectLight = nullptr;
	}
	lctx.set_property(info.Holder(), "_indirectLight", value);
}


void WrapperScene::GetFog(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> fog = lctx.get_property(info.Holder(), "_fog");
	info.GetReturnValue().Set(fog);
}

void WrapperScene::SetFog(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();	
	if (!value->IsNull())
	{
		Fog* fog = lctx.jobj_to_obj<Fog>(value);	
		self->fog = fog;
	}
	else
	{
		self->fog = nullptr;
	}
	lctx.set_property(info.Holder(), "_fog", value);
}

