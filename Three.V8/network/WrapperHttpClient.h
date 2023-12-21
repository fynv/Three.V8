#pragma once

#include "definitions.hpp"

class GameContext;
class WrapperHttpClient
{
public:
	static void define(ObjectDefinition& object);

private:
	static void* ctor();
	static void dtor(void* ptr, GameContext* ctx);
	static void Get(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetAsync(const v8::FunctionCallbackInfo<v8::Value>& info);
};