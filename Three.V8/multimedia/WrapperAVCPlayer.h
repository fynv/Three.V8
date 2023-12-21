#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperAVCPlayer
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void AddPacket(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
};

