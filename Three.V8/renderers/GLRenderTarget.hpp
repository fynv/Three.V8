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
	
};

v8::Local<v8::FunctionTemplate> WrapperGLRenderTarget::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));	
	return templ;
}

void WrapperGLRenderTarget::dtor(void* ptr, GameContext* ctx)
{
	delete (GLRenderTarget*)ptr;
}

void WrapperGLRenderTarget::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	int width = (int)info[0].As<v8::Number>()->Value();
	int height = (int)info[1].As<v8::Number>()->Value();

	bool msaa = true;
	if (info.Length() > 2)
	{
		msaa = info[2].As<v8::Boolean>()->Value();
	}

	GLRenderTarget* self = new GLRenderTarget(false, msaa);
	self->update_framebuffers(width, height);

	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
}
