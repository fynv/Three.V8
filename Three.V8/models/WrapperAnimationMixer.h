#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperAnimationMixer
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void GetAnimations(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetAnimations(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void AddAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void AddAnimations(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void StartAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void StopAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetCurrentPlaying(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetWeights(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetFrame(const v8::FunctionCallbackInfo<v8::Value>& info);

};
