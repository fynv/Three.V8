#include "WrapperUtils.hpp"
#include <renderers/GLRenderTarget.h>
#include "WrapperGLRenderTarget.h"


void WrapperGLRenderTarget::define(ClassDefinition& cls)
{
	cls.name = "GLRenderTarget";
	cls.ctor = ctor;
	cls.dtor = dtor;	
	cls.methods = {
		{"setSize", SetSize },
		{"getImage", GetImage },		
	};
}

void* WrapperGLRenderTarget::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	int width, height;
	lctx.jnum_to_num(info[0], width);
	lctx.jnum_to_num(info[1], height);

	bool msaa = true;
	if (info.Length() > 2)
	{
		msaa = info[2].As<v8::Boolean>()->Value();
	}

	GLRenderTarget* self = new GLRenderTarget(false, msaa);
	self->update_framebuffers(width, height);
	return self;
}

void WrapperGLRenderTarget::dtor(void* ptr, GameContext* ctx)
{
	delete (GLRenderTarget*)ptr;
}

void WrapperGLRenderTarget::SetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderTarget* self = lctx.self<GLRenderTarget>();
	int width, height;
	lctx.jnum_to_num(info[0], width);
	lctx.jnum_to_num(info[1], height);
	self->update_framebuffers(width, height);
}


void WrapperGLRenderTarget::GetImage(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLRenderTarget* self = lctx.self<GLRenderTarget>();
	v8::Local<v8::Object> holder_image = lctx.instantiate("Image");
	Image* image = lctx.jobj_to_obj<Image>(holder_image);
	self->GetImage(*image);
	info.GetReturnValue().Set(holder_image);
}

