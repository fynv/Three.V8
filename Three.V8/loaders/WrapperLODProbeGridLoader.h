#pragma once

#include "definitions.hpp"

class WrapperLODProbeGridLoader
{
public:
	static void define(ObjectDefinition& object);

private:
	static void LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
};


