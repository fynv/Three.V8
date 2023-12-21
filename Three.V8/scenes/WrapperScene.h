#pragma once

#include "definitions.hpp"

class WrapperScene
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetBackground(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetBackground(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetIndirectLight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetIndirectLight(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetFog(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetFog(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetWidgets(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

	static void AddWidget(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RemoveWidget(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void ClearWidgets(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetBoundingBox(const v8::FunctionCallbackInfo<v8::Value>& info);
};

