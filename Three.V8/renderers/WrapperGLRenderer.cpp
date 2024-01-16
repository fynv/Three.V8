#include "WrapperUtils.hpp"
#include <renderers/GLRenderer.h>
#include <scenes/Scene.h>
#include <cameras/Camera.h>
#include <models/GLTFModel.h>
#include <gui/UI3DViewer.h>
#if THREE_MM
#include <MMCamera.h>
#include <MMLazyVideo.h>
#include <MMPlayer.h>
#include <AVCPlayer.h>
#endif

#include "WrapperGLRenderer.h"

void WrapperGLRenderer::define(ClassDefinition& cls)
{
	cls.name = "GLRenderer";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "useSSAO",  GetUseSSAO, SetUseSSAO },		
	};
	cls.methods = {
		{"render", Render },
		{"renderCube", RenderCube },
		{"updateProbe", UpdateProbe },
		{"updateProbes", UpdateProbes },
		{"updateLightmap", UpdateLightmap },
		{"filterLightmap", FilterLightmap },
		{"renderTexture", RenderTexture },
		{"createHeight", CreateHeight },
	};
}

void* WrapperGLRenderer::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new GLRenderer;
}

void WrapperGLRenderer::dtor(void* ptr, GameContext* ctx)
{
	delete (GLRenderer*)ptr;
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

	float zNear = 0.01f;
	float zFar = 100.0f;
	if (info.Length() > 4)
	{
		lctx.jnum_to_num(info[4], zNear);
		lctx.jnum_to_num(info[5], zFar);
	}

	float k = 1.0f;
	if (info.Length() > 6)
	{
		lctx.jnum_to_num(info[6], k);
	}

	v8::Local<v8::Object> holder_grid = info[2].As<v8::Object>();
	std::string clsname = lctx.jstr_to_str(holder_grid->GetConstructorName());

	if (clsname == "ProbeGrid")
	{
		ProbeGrid* probe_grid = lctx.jobj_to_obj<ProbeGrid>(holder_grid);
		glm::ivec3 idx;
		lctx.jvec3_to_ivec3(info[3], idx);
		self->updateProbe(*scene, *target, *probe_grid, idx, zNear, zFar, k);
	}
	else if (clsname == "LODProbeGrid")
	{
		LODProbeGrid* probe_grid = lctx.jobj_to_obj<LODProbeGrid>(holder_grid);
		int idx;
		lctx.jnum_to_num(info[3], idx);
		self->updateProbe(*scene, *target, *probe_grid, idx, zNear, zFar, k);
	}

}

void WrapperGLRenderer::UpdateProbes(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);

	int start_idx = 0;
	int num_directions = 256;
	float rate_vis = 0.5f;
	float rate_irr = 1.0f;
	if (info.Length() > 2)
	{
		lctx.jnum_to_num(info[2], start_idx);
	}
	if (info.Length() > 3)
	{
		lctx.jnum_to_num(info[3], num_directions);
	}
	if (info.Length() > 4)
	{
		lctx.jnum_to_num(info[4], rate_vis);
	}
	if (info.Length() > 5)
	{
		lctx.jnum_to_num(info[5], rate_irr);
	}

	int num_probes;

	v8::Local<v8::Object> holder_grid = info[1].As<v8::Object>();
	std::string clsname = lctx.jstr_to_str(holder_grid->GetConstructorName());

	if (clsname == "ProbeGrid")
	{
		ProbeGrid* probe_grid = lctx.jobj_to_obj<ProbeGrid>(holder_grid);
		num_probes = self->updateProbes(*scene, *probe_grid, start_idx, num_directions, rate_vis, rate_irr);

	}
	else if (clsname == "LODProbeGrid")
	{
		LODProbeGrid* probe_grid = lctx.jobj_to_obj<LODProbeGrid>(holder_grid);
		num_probes = self->updateProbes(*scene, *probe_grid, start_idx, num_directions, rate_vis, rate_irr);
	}
	info.GetReturnValue().Set(lctx.num_to_jnum(num_probes));
}

void WrapperGLRenderer::UpdateLightmap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	GLTFModel* model = lctx.jobj_to_obj<GLTFModel>(info[1]);

	int start_idx = 0;
	int num_directions = 64;
	float k = 1.0f;
	if (info.Length() > 2)
	{
		lctx.jnum_to_num(info[2], start_idx);
	}
	if (info.Length() > 3)
	{
		lctx.jnum_to_num(info[3], num_directions);
	}
	if (info.Length() > 4)
	{
		lctx.jnum_to_num(info[4], k);
	}

	int num_texels = self->updateLightmap(*scene, *model->lightmap, *model->lightmap_target, start_idx, num_directions, k);
	info.GetReturnValue().Set(lctx.num_to_jnum(num_texels));
}


void WrapperGLRenderer::FilterLightmap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	GLTFModel* model = lctx.jobj_to_obj<GLTFModel>(info[0]);
	self->filterLightmap(*model->lightmap, *model->lightmap_target, model->matrixWorld);
}

#if 0
void WrapperGLRenderer::CompressLightmap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	GLTFModel* model = lctx.jobj_to_obj<GLTFModel>(info[1]);
	bool res = self->compressLightmap(*scene, model);
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, res));
}

void WrapperGLRenderer::DecompressLightmap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	GLTFModel* model = lctx.jobj_to_obj<GLTFModel>(info[1]);
	bool res = self->decompressLightmap(*scene, model);
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, res));
}
#endif


void WrapperGLRenderer::GetUseSSAO(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->m_use_ssao));
}

void WrapperGLRenderer::SetUseSSAO(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();
	self->m_use_ssao = value.As<v8::Boolean>()->Value();
}

void WrapperGLRenderer::RenderTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();

	GLTexture2D* tex = nullptr;

	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();
	std::string clsname = lctx.jstr_to_str(holder_image->GetConstructorName());

	bool flipY = true;

	if (clsname == "GLRenderTarget")
	{
		GLRenderTarget* img = lctx.jobj_to_obj<GLRenderTarget>(holder_image);
		if (img != nullptr)
		{
			tex = img->m_tex_video.get();
			flipY = false;
		}
	}

#if THREE_MM
	else if (clsname == "MMCamera")
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
#endif

	int x, y, width, height;
	lctx.jnum_to_num(info[1], x);
	lctx.jnum_to_num(info[2], y);
	lctx.jnum_to_num(info[3], width);
	lctx.jnum_to_num(info[4], height);

	float alpha = 1.0f;
	if (info.Length() > 5)
	{
		lctx.jnum_to_num(info[5], alpha);
	}

	GLRenderTarget* target = nullptr;

	if (info.Length() < 7)
	{
		GamePlayer* player = lctx.player();
		target = &player->renderTarget();
	}
	else
	{
		v8::Local<v8::Object> holder_viewer = info[6].As<v8::Object>();
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

	self->renderTexture(tex, x, y, width, height, *target, flipY, alpha);
}

void WrapperGLRenderer::CreateHeight(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderer* self = lctx.self<GLRenderer>();

	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	v8::Local<v8::Object> global = lctx.context->Global();
	v8::Local<v8::Function> ctor = lctx.get_property(global, "HeightField").As<v8::Function>();
	v8::Local<v8::Value> argv[4] = { info[1], info[2], info[3], info[4] };
	v8::Local<v8::Object> holder = ctor->CallAsConstructor(lctx.context, 4, argv).ToLocalChecked().As<v8::Object>();
	HeightField* height = lctx.jobj_to_obj<HeightField>(holder);

	self->create_height(*scene, height);

	info.GetReturnValue().Set(holder);
}
