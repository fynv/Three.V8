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
	static void dtor(void* ptr);
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


void WrapperEnvironmentMapCreator::dtor(void* ptr)
{
	delete (EnvironmentMapCreator*)ptr;
}

void WrapperEnvironmentMapCreator::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	EnvironmentMapCreator* self = new EnvironmentMapCreator();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
	info.This()->SetInternalField(1, v8::External::New(info.GetIsolate(), dtor));
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This());
}


void WrapperEnvironmentMapCreator::Create(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);

	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Function> ctor_envmap = global->Get(context, v8::String::NewFromUtf8(isolate, "EnvironmentMap").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

	v8::Local<v8::Object> holder = ctor_envmap->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(holder->GetInternalField(0));
	EnvironmentMap* self = (EnvironmentMap*)wrap->Value();
	EnvironmentMapCreator* creator = get_self<EnvironmentMapCreator>(info);


	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();
	
	v8::String::Utf8Value clsname(isolate, holder_image->GetConstructorName());
	if (strcmp(*clsname, "CubeImage") == 0)
	{
		CubeImage* image = (CubeImage*)v8::Local<v8::External>::Cast(holder_image->GetInternalField(0))->Value();
		creator->Create(image, self);
	}
	else if (strcmp(*clsname, "CubeBackground") == 0)
	{
		CubeBackground* background = (CubeBackground*)v8::Local<v8::External>::Cast(holder_image->GetInternalField(0))->Value();
		creator->Create(background, self);
	}

	info.GetReturnValue().Set(holder);
}
