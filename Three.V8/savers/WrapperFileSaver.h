#pragma once

#include "definitions.hpp"

class WrapperFileSaver
{
public:
	static void define(ObjectDefinition& object);

private:
	static void SaveBinaryFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SaveTextFile(const v8::FunctionCallbackInfo<v8::Value>& info);

};
