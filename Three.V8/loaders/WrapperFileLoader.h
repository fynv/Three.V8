#pragma once

#include "definitions.hpp"

class WrapperFileLoader
{
public:
	static void define(ObjectDefinition& object);

private:
	static void LoadBinaryFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadTextFile(const v8::FunctionCallbackInfo<v8::Value>& info);
};



