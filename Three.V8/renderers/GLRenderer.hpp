#pragma once

#include "WrapperUtils.hpp"
#include <renderers/GLRenderer.h>
#include <scenes/Scene.h>
#include <cameras/Camera.h>
#include <gui/UI3DViewer.h>
#include "GamePlayer.h"

class WrapperGLRenderer
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void Render(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RenderCube(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RenderCelluloid(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperGLRenderer::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "render", v8::FunctionTemplate::New(isolate, Render));
	templ->InstanceTemplate()->Set(isolate, "renderCube", v8::FunctionTemplate::New(isolate, RenderCube));
	templ->InstanceTemplate()->Set(isolate, "renderCelluloid", v8::FunctionTemplate::New(isolate, RenderCelluloid));
	return templ;
}

void WrapperGLRenderer::dtor(void* ptr, GameContext* ctx)
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

	v8::Local<v8::Object> holder_scene = info[0].As<v8::Object>();
	Scene* scene = (Scene*)holder_scene->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_camera = info[1].As<v8::Object>();
	Camera* camera = (Camera*)holder_camera->GetAlignedPointerFromInternalField(0);

	if (info.Length() < 3)
	{
		v8::Local<v8::Object> global = context->Global();
		v8::Local<v8::Object> holder_player = global->Get(context, v8::String::NewFromUtf8(isolate, "gamePlayer").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
		GamePlayer* player = (GamePlayer*)holder_player->GetAlignedPointerFromInternalField(0);
		self->render(*scene, *camera, player->renderTarget());
	}
	else
	{
		v8::Local<v8::Object> holder_viewer = info[2].As<v8::Object>();

		v8::String::Utf8Value clsname(isolate, holder_viewer->GetConstructorName());
		if (strcmp(*clsname, "UI3DViewer") == 0)
		{
			UI3DViewer* viewer = (UI3DViewer*)holder_viewer->GetAlignedPointerFromInternalField(0);
			self->render(*scene, *camera, viewer->render_target);
		}
		else if (strcmp(*clsname, "GLRenderTarget") == 0)
		{
			GLRenderTarget* target = (GLRenderTarget*)holder_viewer->GetAlignedPointerFromInternalField(0);
			self->render(*scene, *camera, *target);
		}
	}
}

void WrapperGLRenderer::RenderCube(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	GLRenderer* self = get_self<GLRenderer>(info);

	v8::Local<v8::Object> holder_scene = info[0].As<v8::Object>();
	Scene* scene = (Scene*)holder_scene->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_target = info[1].As<v8::Object>();
	CubeRenderTarget* target = (CubeRenderTarget*)holder_target->GetAlignedPointerFromInternalField(0);
	
	v8::Local<v8::Object> in = info[2].As<v8::Object>();
	glm::vec3 position;
	jvec3_to_vec3(isolate, in, position);

	float zNear = 0.1f;
	float zFar = 100.0f;
	if (info.Length() > 4)
	{
		zNear = (float)info[3].As<v8::Number>()->Value();
		zFar = (float)info[4].As<v8::Number>()->Value();
	}

	self->renderCube(*scene, *target, position, zNear, zFar);

}

void WrapperGLRenderer::RenderCelluloid(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	GLRenderer* self = get_self<GLRenderer>(info);

	v8::Local<v8::Object> holder_scene = info[0].As<v8::Object>();
	Scene* scene = (Scene*)holder_scene->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_camera = info[1].As<v8::Object>();
	Camera* camera = (Camera*)holder_camera->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_base = info[2].As<v8::Object>();
	GLRenderTarget* target_base = (GLRenderTarget*)holder_base->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_lighting = info[3].As<v8::Object>();
	GLRenderTarget* target_lighting = (GLRenderTarget*)holder_lighting->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_alpha = info[4].As<v8::Object>();
	GLRenderTarget* target_alpha = (GLRenderTarget*)holder_alpha->GetAlignedPointerFromInternalField(0);

	self->renderCelluloid(*scene, *camera, target_base, target_lighting, target_alpha);


}