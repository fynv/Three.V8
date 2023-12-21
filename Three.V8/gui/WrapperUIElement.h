#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperUIElement
{
public:
	static void define(ClassDefinition& cls);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void GetBlock(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetBlock(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetOnPointerDown(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnPointerDown(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOnPointerUp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnPointerUp(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOnPointerMove(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnPointerMove(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};
