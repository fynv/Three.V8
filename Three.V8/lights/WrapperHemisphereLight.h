#pragma once

#include "definitions.hpp"


class WrapperHemisphereLight
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetSkyColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetGroundColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};

