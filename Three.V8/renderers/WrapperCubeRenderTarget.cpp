#include "WrapperUtils.hpp"
#include <renderers/CubeRenderTarget.h>
#include "WrapperCubeRenderTarget.h"


void WrapperCubeRenderTarget::define(ClassDefinition& cls)
{
	cls.name = "CubeRenderTarget";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.methods = {
		{"getCubeImage", GetCubeImage },
		{"getHDRCubeImage", GetHDRCubeImage },
	};
}

void* WrapperCubeRenderTarget::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	int width, height;
	lctx.jnum_to_num(info[0], width);
	lctx.jnum_to_num(info[1], height);

	CubeRenderTarget* self = new CubeRenderTarget();
	self->update_framebuffers(width, height);
	return self;
}

void WrapperCubeRenderTarget::dtor(void* ptr, GameContext* ctx)
{
	delete (CubeRenderTarget*)ptr;
}

void WrapperCubeRenderTarget::GetCubeImage(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeRenderTarget* self = lctx.self<CubeRenderTarget>();
	v8::Local<v8::Object> holder_image = lctx.instantiate("CubeImage");
	CubeImage* image = lctx.jobj_to_obj<CubeImage>(holder_image);
	self->GetCubeImage(*image);
	info.GetReturnValue().Set(holder_image);
}

void WrapperCubeRenderTarget::GetHDRCubeImage(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeRenderTarget* self = lctx.self<CubeRenderTarget>();
	v8::Local<v8::Object> holder_image = lctx.instantiate("HDRCubeImage");
	HDRCubeImage* image = lctx.jobj_to_obj<HDRCubeImage>(holder_image);
	self->GetHDRCubeImage(*image);
	info.GetReturnValue().Set(holder_image);
}


