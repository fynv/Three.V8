#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperEnvironmentMapCreator
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

	static void Create(const v8::FunctionCallbackInfo<v8::Value>& info);
};

