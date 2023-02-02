#pragma once

#include "WrapperUtils.hpp"
#include <backgrounds/BackgroundScene.h>

class WrapperBackgroundScene
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);

	static void GetScene(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetScene(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetNear(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetNear(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetFar(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetFar(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};


v8::Local<v8::FunctionTemplate> WrapperBackgroundScene::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "scene").ToLocalChecked(), GetScene, SetScene);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "near").ToLocalChecked(), GetNear, SetNear);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "far").ToLocalChecked(), GetFar, SetFar);
	return templ;
}

void WrapperBackgroundScene::dtor(void* ptr, GameContext* ctx)
{
	delete (BackgroundScene*)ptr;
}


void WrapperBackgroundScene::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Scene* scene = nullptr;
	float z_near = 10.0f; 
	float z_far = 10000.0f;

	if (info.Length() > 0)
	{
		if (!info[0]->IsNull())
		{
			scene = lctx.jobj_to_obj<Scene>(info[0]);
		}
		if (info.Length() > 1)
		{
			lctx.jnum_to_num(info[1], z_near);
			if (info.Length() > 2)
			{
				lctx.jnum_to_num(info[2], z_far);
			}
		}
	}
	BackgroundScene* self = new BackgroundScene(scene, z_near, z_far);
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}

void WrapperBackgroundScene::GetScene(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> scene = lctx.get_property(info.Holder(), "_scene");
	info.GetReturnValue().Set(scene);
}

void WrapperBackgroundScene::SetScene(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(info.Holder(), "_scene", value);

	BackgroundScene* self = lctx.self<BackgroundScene>();
	Scene* scene = nullptr;
	if (!value->IsNull())
	{
		lctx.jobj_to_obj<Scene>(value);
	}
	self->scene = scene;
}


void WrapperBackgroundScene::GetNear(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	BackgroundScene* self = lctx.self<BackgroundScene>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->z_near));
}

void WrapperBackgroundScene::SetNear(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	BackgroundScene* self = lctx.self<BackgroundScene>();
	lctx.jnum_to_num(value, self->z_near);
}


void WrapperBackgroundScene::GetFar(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	BackgroundScene* self = lctx.self<BackgroundScene>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->z_far));
}

void WrapperBackgroundScene::SetFar(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	BackgroundScene* self = lctx.self<BackgroundScene>();
	lctx.jnum_to_num(value, self->z_far);
}

