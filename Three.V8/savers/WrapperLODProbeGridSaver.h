#pragma once

#include "definitions.hpp"

class WrapperLODProbeGridSaver
{
public:
	static void define(ObjectDefinition& object);

private:
	static void SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info);
};
