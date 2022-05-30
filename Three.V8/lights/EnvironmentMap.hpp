#pragma once

#include "WrapperUtils.hpp"
#include "IndirectLight.hpp"
#include <lights/EnvironmentMap.h>
#include <utils/Image.h>

class WrapperEnvironmentMap
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperEnvironmentMap::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperIndirectLight::create_template(isolate, constructor);	
	return templ;
}

void WrapperEnvironmentMap::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	EnvironmentMap* self = new EnvironmentMap();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
	info.This()->SetInternalField(1, v8::External::New(info.GetIsolate(), WrapperIndirectLight::dtor));
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This());
}
