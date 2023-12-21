#pragma once

#include "definitions.hpp"

class WrapperVolumeDataLoader
{
public:
	static void define(ObjectDefinition& object);

private:
	static void LoadRawVolumeFile(const v8::FunctionCallbackInfo<v8::Value>& info);

};

