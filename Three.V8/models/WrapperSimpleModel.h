#pragma once

#include "definitions.hpp"

class WrapperSimpleModel
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void CreateBox(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreateSphere(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreatePlane(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void SetColorTexture(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetMetalness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetMetalness(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetRoughness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetRoughness(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void SetToonShading(const v8::FunctionCallbackInfo<v8::Value>& info);
};

