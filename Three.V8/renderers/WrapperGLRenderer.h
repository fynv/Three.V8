#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperGLRenderer
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void Render(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RenderCube(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void UpdateProbe(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void UpdateProbes(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void UpdateLightmap(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void FilterLightmap(const v8::FunctionCallbackInfo<v8::Value>& info);
	//	static void CompressLightmap(const v8::FunctionCallbackInfo<v8::Value>& info);
	//	static void DecompressLightmap(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetUseSSAO(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetUseSSAO(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void RenderTexture(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void CreateHeight(const v8::FunctionCallbackInfo<v8::Value>& info);
};

