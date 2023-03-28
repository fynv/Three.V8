#pragma once

#include "WrapperUtils.hpp"
#include <renderers/CubeRenderTarget.h>


class WrapperCubeRenderTarget
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void GetCubeImage(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetHDRCubeImage(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperCubeRenderTarget::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));	
	templ->InstanceTemplate()->Set(isolate, "getCubeImage", v8::FunctionTemplate::New(isolate, GetCubeImage));
	templ->InstanceTemplate()->Set(isolate, "getHDRCubeImage", v8::FunctionTemplate::New(isolate, GetHDRCubeImage));
	return templ;
}

void WrapperCubeRenderTarget::dtor(void* ptr, GameContext* ctx)
{
	delete (CubeRenderTarget*)ptr;
}

void WrapperCubeRenderTarget::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	int width, height;
	lctx.jnum_to_num(info[0], width);
	lctx.jnum_to_num(info[1], height);

	CubeRenderTarget* self = new CubeRenderTarget();
	self->update_framebuffers(width, height);

	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
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
