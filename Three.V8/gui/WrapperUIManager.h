#pragma once

#include "definitions.hpp"

class WrapperUIManager
{
public:
	static void define(ObjectDefinition& object);

private:
	static void* ctor();
	static void dtor(void* ptr, GameContext* ctx);
	static void GetAreas(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Add(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Remove(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Clear(const v8::FunctionCallbackInfo<v8::Value>& info);
};

