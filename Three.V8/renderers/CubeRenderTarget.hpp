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
};

v8::Local<v8::FunctionTemplate> WrapperCubeRenderTarget::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));	
	templ->InstanceTemplate()->Set(isolate, "getCubeImage", v8::FunctionTemplate::New(isolate, GetCubeImage));
	return templ;
}

void WrapperCubeRenderTarget::dtor(void* ptr, GameContext* ctx)
{
	delete (CubeRenderTarget*)ptr;
}

void WrapperCubeRenderTarget::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	int width = (int)info[0].As<v8::Number>()->Value();
	int height = (int)info[1].As<v8::Number>()->Value();

	CubeRenderTarget* self = new CubeRenderTarget();
	self->update_framebuffers(width, height);

	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
}

void WrapperCubeRenderTarget::GetCubeImage(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Function> ctor_image = global->Get(context, v8::String::NewFromUtf8(isolate, "CubeImage").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

	v8::Local<v8::Object> holder_image = ctor_image->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	CubeImage* image = (CubeImage*)holder_image->GetAlignedPointerFromInternalField(0);

	CubeRenderTarget* self = get_self<CubeRenderTarget>(info);

	self->GetCubeImage(*image);
	info.GetReturnValue().Set(holder_image);

}
