#pragma once

#include "WrapperUtils.hpp"
#include <lights/EnvironmentMapCreator.h>
#include <utils/Image.h>

class WrapperEnvironmentMapCreator
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void Create(const v8::FunctionCallbackInfo<v8::Value>& info);

};


v8::Local<v8::FunctionTemplate> WrapperEnvironmentMapCreator::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "create", v8::FunctionTemplate::New(isolate, Create));
	return templ;
}


void WrapperEnvironmentMapCreator::dtor(void* ptr, GameContext* ctx)
{
	delete (EnvironmentMapCreator*)ptr;
}

void WrapperEnvironmentMapCreator::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	EnvironmentMapCreator* self = new EnvironmentMapCreator();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
}


void WrapperEnvironmentMapCreator::Create(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);

	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Function> ctor_envmap = global->Get(context, v8::String::NewFromUtf8(isolate, "EnvironmentMap").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

	v8::Local<v8::Object> holder = ctor_envmap->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();	
	EnvironmentMap* self = (EnvironmentMap*)holder->GetAlignedPointerFromInternalField(0);
	EnvironmentMapCreator* creator = get_self<EnvironmentMapCreator>(info);


	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();
	
	v8::String::Utf8Value clsname(isolate, holder_image->GetConstructorName());
	if (strcmp(*clsname, "CubeImage") == 0)
	{
		CubeImage* image = (CubeImage*)holder_image->GetAlignedPointerFromInternalField(0);
		creator->Create(image, self);
	}
	else if (strcmp(*clsname, "CubeBackground") == 0)
	{
		CubeBackground* background = (CubeBackground*)holder_image->GetAlignedPointerFromInternalField(0);
		creator->Create(background, self);
	}

	info.GetReturnValue().Set(holder);
}
