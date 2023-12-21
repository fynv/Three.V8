#pragma once

#include "definitions.hpp"

class WrapperCamera
{
public:
	static void define(ClassDefinition& cls);	

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetMatrixWorldInverse(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetMatrixWorldInverse(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetProjectionMatrix(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetProjectionMatrix(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetProjectionMatrixInverse(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetProjectionMatrixInverse(const v8::FunctionCallbackInfo<v8::Value>& info);

};

