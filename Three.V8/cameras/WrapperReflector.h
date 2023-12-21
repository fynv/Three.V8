#pragma once

#include "definitions.hpp"

class WrapperReflector
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetWidth(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetHeight(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetPrimitiveReferences(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void AddPrimitiveReference(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void ClearPrimitiveReferences(const v8::FunctionCallbackInfo<v8::Value>& info);

};

