#pragma once

#include "WrapperUtils.hpp"
#include <renderers/GLRenderer.h>
#include <scenes/Scene.h>
#include <cameras/Camera.h>
#include "GamePlayer.h"

class WrapperGLRenderer
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr);
	static void Render(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperGLRenderer::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "render", v8::FunctionTemplate::New(isolate, Render));
	return templ;
}

void WrapperGLRenderer::dtor(void* ptr)
{
	delete (GLRenderer*)ptr;
}

void WrapperGLRenderer::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	GLRenderer* self = new GLRenderer();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
}

void WrapperGLRenderer::Render(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	
	GLRenderer* self = get_self<GLRenderer>(info);

	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Object> holder_player = global->Get(context, v8::String::NewFromUtf8(isolate, "gamePlayer").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
	GamePlayer* player = (GamePlayer*)holder_player->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_scene = info[0].As<v8::Object>();
	Scene* scene = (Scene*)holder_scene->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_camera = info[1].As<v8::Object>();
	Camera* camera = (Camera*)holder_camera->GetAlignedPointerFromInternalField(0);

	self->render(*scene, *camera, player->renderTarget());
}