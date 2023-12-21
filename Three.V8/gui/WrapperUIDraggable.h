#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperUIDraggable
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void GetDraggableHorizontal(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetDraggableHorizontal(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
	static void GetDraggableVertical(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetDraggableVertical(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOriginMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetOriginMin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetOriginMin(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetOriginMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetOriginMax(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetOriginMax(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetValue(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetValue(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetValue(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetOnDrag(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnDrag(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};
