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
	LocalContext lctx(info);
	EnvironmentMapCreator* self = new EnvironmentMapCreator();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
}


void WrapperEnvironmentMapCreator::Create(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	EnvironmentMapCreator* creator = lctx.self<EnvironmentMapCreator>();

	v8::Local<v8::Object> holder = lctx.instantiate("EnvironmentMap");
	EnvironmentMap* self = lctx.jobj_to_obj<EnvironmentMap>(holder);

	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();
	
	std::string clsname = lctx.jstr_to_str(holder_image->GetConstructorName());
	if (clsname == "CubeImage")
	{
		CubeImage* image = lctx.jobj_to_obj<CubeImage>(holder_image);
		creator->Create(image, self);
	}
	else if (clsname == "CubeBackground")
	{
		CubeBackground* background = lctx.jobj_to_obj<CubeBackground>(holder_image);
		creator->Create(background, self);
	}
	else  if (clsname == "CubeRenderTarget")
	{
		CubeRenderTarget* target = lctx.jobj_to_obj<CubeRenderTarget>(holder_image);		
		creator->Create(target, self);
	}

	info.GetReturnValue().Set(holder);
}
