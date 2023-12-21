#pragma once

#include "definitions.hpp"

class WrapperGLTFLoader
{
public:
	static void define(ObjectDefinition& object);

private:
	static void LoadModelFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadAnimationsFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadModelFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadAnimationsFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
};

