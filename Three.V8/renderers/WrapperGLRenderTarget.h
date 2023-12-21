#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperGLRenderTarget
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void SetSize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetImage(const v8::FunctionCallbackInfo<v8::Value>& info);
};
