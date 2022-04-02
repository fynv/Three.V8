#pragma once

#include "WrapperUtils.hpp"
#include <renderers/GLRenderer.h>
#include <scenes/Scene.h>
#include <cameras/Camera.h>

class WrapperGLRenderer
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void Dispose(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Render(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperGLRenderer::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(1);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, Dispose));
	templ->InstanceTemplate()->Set(isolate, "render", v8::FunctionTemplate::New(isolate, Render));
	return templ;
}

void WrapperGLRenderer::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	GLRenderer* self = new GLRenderer();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
}

void WrapperGLRenderer::Dispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	GLRenderer* self = get_self<GLRenderer>(info);
	delete self;
}

void WrapperGLRenderer::Render(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	GLRenderer* self = get_self<GLRenderer>(info);

	int width = (int)info[0].As<v8::Number>()->Value();
	int height = (int)info[1].As<v8::Number>()->Value();

	v8::Local<v8::Object> holder_scene = info[2].As<v8::Object>();
	Scene* scene = (Scene*)v8::Local<v8::External>::Cast(holder_scene->GetInternalField(0))->Value();

	v8::Local<v8::Object> holder_camera = info[3].As<v8::Object>();
	Camera* camera = (Camera*)v8::Local<v8::External>::Cast(holder_camera->GetInternalField(0))->Value();

	self->render(width, height, *scene, *camera);
}