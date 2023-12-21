#pragma once

#include "definitions.hpp"

class WrapperGLTFModel
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetMinPos(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetMaxPos(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

	static void GetMeshes(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

	static void SetTexture(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void BatchPrimitives(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void SetAnimationFrame(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetAnimations(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetAnimations(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void AddAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void AddAnimations(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void PlayAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void StopAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void UpdateAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void SetToonShading(const v8::FunctionCallbackInfo<v8::Value>& info);

	// lightmaps
	static void GetIsBakable(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetLightmap(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetLightmap(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void InitializeLightmap(const v8::FunctionCallbackInfo<v8::Value>& info);
};


