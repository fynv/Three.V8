#pragma once

#include "definitions.hpp"


class WrapperUIPanel
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info);
};

