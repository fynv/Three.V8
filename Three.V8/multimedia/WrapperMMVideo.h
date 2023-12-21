#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperMMVideo
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetLooping(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetLooping(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void IsPlaying(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetDuration(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Play(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Pause(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetPosition(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetAudioDevice(const v8::FunctionCallbackInfo<v8::Value>& info);
};
