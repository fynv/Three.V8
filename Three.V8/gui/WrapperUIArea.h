#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperUIArea
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void GetElements(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Add(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Remove(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Clear(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetViewers(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void AddViewer(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RemoveViewer(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void ClearViewers(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetSize(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetScale(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetScale(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};
