#pragma once

#include "definitions.hpp"

class WrapperDirectionalLight
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetTarget(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetTarget(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
	static void SetShadow(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetShadowProjection(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetShadowRadius(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetBias(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetBias(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetForceCull(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetForceCull(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetBoundingBox(const v8::FunctionCallbackInfo<v8::Value>& info);
};


