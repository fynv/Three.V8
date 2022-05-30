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
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), WrapperIndirectLight::dtor);
}
