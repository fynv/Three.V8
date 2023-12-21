#pragma once

#include "definitions.hpp"

class WrapperEnvironmentMap
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetSHCoefficients(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetSHCoefficients(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};

