#pragma once

#include "definitions.hpp"

class WrapperHDRImageLoader
{
public:
	static void define(ObjectDefinition& object);

private:
	static void LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadCubeFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadCubeFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void FromImages(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void FromRGBM(const v8::FunctionCallbackInfo<v8::Value>& info);
};
