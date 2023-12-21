#pragma once

#include "definitions.hpp"

class WrapperText
{
public:
	static void define(ObjectDefinition& object);

private:
	static void ToUTF8Buffer(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void FromUTF8Buffer(const v8::FunctionCallbackInfo<v8::Value>& info);

};

