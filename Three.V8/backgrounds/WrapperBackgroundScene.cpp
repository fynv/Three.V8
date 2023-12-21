#include "WrapperUtils.hpp"
#include <backgrounds/BackgroundScene.h>

#include "WrapperBackgroundScene.h"


void WrapperBackgroundScene::define(ClassDefinition& cls)
{
	cls.name = "BackgroundScene";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "scene",  GetScene, SetScene },
		{ "near",  GetNear, SetNear },
		{ "far",  GetFar, SetFar },
	};
}

void* WrapperBackgroundScene::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Scene* scene = nullptr;
	float z_near = 10.0f;
	float z_far = 10000.0f;

	if (info.Length() > 0)
	{
		LocalContext lctx(info);
		lctx.set_property(info.This(), "_scene", info[0]);

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
	return new BackgroundScene(scene, z_near, z_far);
}

void WrapperBackgroundScene::dtor(void* ptr, GameContext* ctx)
{
	delete (BackgroundScene*)ptr;
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
		scene = lctx.jobj_to_obj<Scene>(value);
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

