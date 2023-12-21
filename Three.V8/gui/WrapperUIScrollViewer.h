#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperUIScrollViewer
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetScrollableVertical(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetScrollableVertical(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetScrollableHorizontal(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetScrollableHorizontal(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetScrollPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetScrollPosition(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetScrollPosition(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetContentSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetContentSize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetContentSize(const v8::FunctionCallbackInfo<v8::Value>& info);
};

