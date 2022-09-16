#pragma once

#include "WrapperUtils.hpp"
#include <renderers/GLRenderer.h>
#include <scenes/Scene.h>
#include <cameras/Camera.h>
#include <gui/UI3DViewer.h>

class WrapperGLRenderer
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void Render(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RenderCube(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RenderLayers(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RenderLayersToCube(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RenderCelluloid(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperGLRenderer::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "render", v8::FunctionTemplate::New(isolate, Render));
	templ->InstanceTemplate()->Set(isolate, "renderCube", v8::FunctionTemplate::New(isolate, RenderCube));

	templ->InstanceTemplate()->Set(isolate, "renderLayers", v8::FunctionTemplate::New(isolate, RenderLayers));
	templ->InstanceTemplate()->Set(isolate, "renderLayersToCube", v8::FunctionTemplate::New(isolate, RenderLayersToCube));

	templ->InstanceTemplate()->Set(isolate, "renderCelluloid", v8::FunctionTemplate::New(isolate, RenderCelluloid));
	return templ;
}

void WrapperGLRenderer::dtor(void* ptr, GameContext* ctx)
{
	delete (GLRenderer*)ptr;
}

void WrapperGLRenderer::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = new GLRenderer();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
}

void WrapperGLRenderer::Render(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	Camera* camera = lctx.jobj_to_obj<Camera>(info[1]);

	if (info.Length() < 3)
	{		
		GamePlayer* player = lctx.player();	
		self->render(*scene, *camera, player->renderTarget());
	}
	else
	{
		v8::Local<v8::Object> holder_viewer = info[2].As<v8::Object>();
		std::string clsname = lctx.jstr_to_str(holder_viewer->GetConstructorName());
		if (clsname == "UI3DViewer")
		{
			UI3DViewer* viewer = lctx.jobj_to_obj<UI3DViewer>(holder_viewer);
			self->render(*scene, *camera, viewer->render_target);
		}
		else if (clsname == "GLRenderTarget")
		{
			GLRenderTarget* target = lctx.jobj_to_obj<GLRenderTarget>(holder_viewer);			
			self->render(*scene, *camera, *target);
		}
	}
}

void WrapperGLRenderer::RenderCube(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	CubeRenderTarget* target = lctx.jobj_to_obj<CubeRenderTarget>(info[1]);

	glm::vec3 position;
	lctx.jvec3_to_vec3(info[2], position);

	float zNear = 0.1f;
	float zFar = 100.0f;
	if (info.Length() > 4)
	{
		lctx.jnum_to_num(info[3], zNear);
		lctx.jnum_to_num(info[4], zFar);
	}

	self->renderCube(*scene, *target, position, zNear, zFar);

}

void WrapperGLRenderer::RenderLayers(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	GLRenderTarget* target = nullptr;

	if (info.Length() < 2)
	{		
		GamePlayer* player = lctx.player();
		target = &player->renderTarget();
	}
	else
	{
		v8::Local<v8::Object> holder_viewer = info[1].As<v8::Object>();
		std::string clsname = lctx.jstr_to_str(holder_viewer->GetConstructorName());
		if (clsname == "UI3DViewer")
		{
			UI3DViewer* viewer = lctx.jobj_to_obj<UI3DViewer>(holder_viewer);
			target = &viewer->render_target;
		}
		else if (clsname == "GLRenderTarget")
		{
			target = lctx.jobj_to_obj<GLRenderTarget>(holder_viewer);			
		}
	}

	v8::Local<v8::Array> o_layers = info[0].As<v8::Array>();
	std::vector<GLRenderer::Layer> layers(o_layers->Length());
	for (int i = 0; i < o_layers->Length(); i++)
	{
		v8::Local<v8::Object> o_layer = o_layers->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		v8::Local<v8::Value> holder_scene = lctx.get_property(o_layer, "scene");
		v8::Local<v8::Value> holder_camera = lctx.get_property(o_layer, "camera");
		layers[i].scene = lctx.jobj_to_obj<Scene>(holder_scene);
		layers[i].camera = lctx.jobj_to_obj<Camera>(holder_camera);
	}

	self->renderLayers(layers.size(), layers.data(), *target);
}

void WrapperGLRenderer::RenderLayersToCube(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();

	v8::Local<v8::Array> o_layers = info[0].As<v8::Array>();
	std::vector<GLRenderer::CubeLayer> layers(o_layers->Length());
	for (int i = 0; i < o_layers->Length(); i++)
	{
		v8::Local<v8::Object> o_layer = o_layers->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		v8::Local<v8::Value> holder_scene = lctx.get_property(o_layer, "scene");
		v8::Local<v8::Value> jpos = lctx.get_property(o_layer, "position");

		layers[i].scene = lctx.jobj_to_obj<Scene>(holder_scene);
		lctx.jvec3_to_vec3(jpos, layers[i].position);
		lctx.jnum_to_num(lctx.get_property(o_layer, "near"), layers[i].zNear);
		lctx.jnum_to_num(lctx.get_property(o_layer, "far"), layers[i].zFar);
	}

	CubeRenderTarget* target = lctx.jobj_to_obj<CubeRenderTarget>(info[1]);
	self->renderLayersToCube(layers.size(), layers.data(), *target);
}

void WrapperGLRenderer::RenderCelluloid(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	Camera* camera = lctx.jobj_to_obj<Camera>(info[1]);
	GLRenderTarget* target_base = lctx.jobj_to_obj<GLRenderTarget>(info[2]);
	GLRenderTarget* target_lighting = lctx.jobj_to_obj<GLRenderTarget>(info[3]);
	GLRenderTarget* target_alpha = lctx.jobj_to_obj<GLRenderTarget>(info[4]);
	self->renderCelluloid(*scene, *camera, target_base, target_lighting, target_alpha);
}
