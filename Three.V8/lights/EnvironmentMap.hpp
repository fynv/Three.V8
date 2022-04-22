#pragma once

#include "WrapperUtils.hpp"
#include <lights/EnvironmentMap.h>
#include <utils/Image.h>

class WrapperEnvironmentMap
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void Dispose(const v8::FunctionCallbackInfo<v8::Value>& info);


};

v8::Local<v8::FunctionTemplate> WrapperEnvironmentMap::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(1);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, Dispose));
	return templ;
}

void WrapperEnvironmentMap::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	EnvironmentMap* self = new EnvironmentMap();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
}

void WrapperEnvironmentMap::Dispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	EnvironmentMap* self = get_self<EnvironmentMap>(info);
	delete self;
}
