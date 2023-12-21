#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperBoundingVolumeHierarchy
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void Update(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Remove(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Intersect(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Collide(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info);

#if ENABLE_TEST
	static void Test(const v8::FunctionCallbackInfo<v8::Value>& info);
#endif

};

