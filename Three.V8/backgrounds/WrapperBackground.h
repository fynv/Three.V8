#pragma once

#include "definitions.hpp"

class GameContext;


class WrapperColorBackground
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);
	
	static void GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
};

class WrapperCubeBackground
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void SetCubemap(const v8::FunctionCallbackInfo<v8::Value>& info);
};


class WrapperHemisphereBackground
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void GetSkyColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetGroundColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info);

};


