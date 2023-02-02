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


};


v8::Local<v8::FunctionTemplate> WrapperBackgroundScene::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	return templ;
}

void WrapperBackgroundScene::dtor(void* ptr, GameContext* ctx)
{
	delete (BackgroundScene*)ptr;
}


void WrapperBackgroundScene::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	float z_near = 10.0f; 
	float z_far = 10000.0f;
	if (info.Length() > 1)
	{
		lctx.jnum_to_num(info[1], z_near);
		if (info.Length() > 2)
		{
			lctx.jnum_to_num(info[2], z_far);
		}
	}
	BackgroundScene* self = new BackgroundScene(scene, z_near, z_far);
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}