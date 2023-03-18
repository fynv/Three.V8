#pragma once

#include "WrapperUtils.hpp"
#include <renderers/GLRenderTarget.h>


class WrapperGLRenderTarget
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void SetSize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetImage(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperGLRenderTarget::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));	
	templ->InstanceTemplate()->Set(isolate, "setSize", v8::FunctionTemplate::New(isolate, SetSize));
	templ->InstanceTemplate()->Set(isolate, "getImage", v8::FunctionTemplate::New(isolate, GetImage));
	return templ;
}

void WrapperGLRenderTarget::dtor(void* ptr, GameContext* ctx)
{
	delete (GLRenderTarget*)ptr;
}

void WrapperGLRenderTarget::New(const v8::FunctionCallbackInfo<v8::Value>& info)
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

	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
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
