#pragma once

#include "definitions.hpp"


class WrapperLODProbeGridWidget
{
public:
	static void define(ClassDefinition& cls);

private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetCoverageMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetCoverageMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetBaseDivisions(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetProbeGrid(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetProbeGrid(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};

