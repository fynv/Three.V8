#pragma once

#include "WrapperUtils.hpp"
#include <renderers/GLRenderer.h>
#include <scenes/Scene.h>
#include <cameras/Camera.h>
#include <gui/UI3DViewer.h>
#if THREE_MM
#include <MMCamera.h>
#include <MMLazyVideo.h>
#include <MMPlayer.h>
#include <AVCPlayer.h>
#endif

class WrapperGLRenderer
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void Render(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RenderCube(const v8::FunctionCallbackInfo<v8::Value>& info);	
	static void UpdateProbe(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RenderCelluloid(const v8::FunctionCallbackInfo<v8::Value>& info);
#if THREE_MM
	static void RenderTexture(const v8::FunctionCallbackInfo<v8::Value>& info);
#endif
};

v8::Local<v8::FunctionTemplate> WrapperGLRenderer::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "render", v8::FunctionTemplate::New(isolate, Render));
	templ->InstanceTemplate()->Set(isolate, "renderCube", v8::FunctionTemplate::New(isolate, RenderCube));
	templ->InstanceTemplate()->Set(isolate, "updateProbe", v8::FunctionTemplate::New(isolate, UpdateProbe));

	templ->InstanceTemplate()->Set(isolate, "renderCelluloid", v8::FunctionTemplate::New(isolate, RenderCelluloid));
#if THREE_MM
	templ->InstanceTemplate()->Set(isolate, "renderTexture", v8::FunctionTemplate::New(isolate, RenderTexture));
#endif
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

		if (player->Picking())
		{
			self->render_picking(*scene, *camera, *player->pickingTarget());
		}
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


void WrapperGLRenderer::UpdateProbe(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	CubeRenderTarget* target = lctx.jobj_to_obj<CubeRenderTarget>(info[1]);
	ProbeGrid* probe_grid = lctx.jobj_to_obj<ProbeGrid>(info[2]);
	glm::ivec3 idx;
	lctx.jvec3_to_ivec3(info[3], idx);

	float zNear = 0.1f;
	float zFar = 100.0f;
	if (info.Length() > 4)
	{
		lctx.jnum_to_num(info[4], zNear);
		lctx.jnum_to_num(info[5], zFar);
	}

	self->updateProbe(*scene, *target, *probe_grid, idx, zNear, zFar);

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

#if THREE_MM
void WrapperGLRenderer::RenderTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();

	GLTexture2D* tex = nullptr;

	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();
	std::string clsname = lctx.jstr_to_str(holder_image->GetConstructorName());

	if (clsname == "MMCamera")
	{
		MMCamera* cam = lctx.jobj_to_obj<MMCamera>(holder_image);
		if (cam != nullptr)
		{
			tex = cam->get_texture();
		}
	}
	else if (clsname == "MMLazyVideo")
	{
		MMLazyVideo* video = lctx.jobj_to_obj<MMLazyVideo>(holder_image);
		if (video != nullptr)
		{
			tex = video->get_texture();
		}
	}
	else if (clsname == "MMVideo")
	{
		MMVideo* video = lctx.jobj_to_obj<MMVideo>(holder_image);
		if (video != nullptr)
		{
			tex = video->get_texture();
		}
	}
	else if (clsname == "AVCPlayer")
	{
		AVCPlayer* player = lctx.jobj_to_obj<AVCPlayer>(holder_image);
		if (player != nullptr)
		{
			tex = player->get_texture();
		}
	}

	int x, y, width, height;
	lctx.jnum_to_num(info[1], x);
	lctx.jnum_to_num(info[2], y);
	lctx.jnum_to_num(info[3], width);
	lctx.jnum_to_num(info[4], height);

	GLRenderTarget* target = nullptr;

	if (info.Length() < 6)
	{
		GamePlayer* player = lctx.player();
		target = &player->renderTarget();
	}
	else
	{
		v8::Local<v8::Object> holder_viewer = info[5].As<v8::Object>();
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
	
	self->renderTexture(tex, x, y, width, height, *target);
}
#endif