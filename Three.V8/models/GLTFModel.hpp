#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <models/GLTFModel.h>
#include <models/GeometryCreator.h>
#include <utils/Image.h>

class WrapperGLTFModel
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperGLTFModel::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	return templ;
}

void WrapperGLTFModel::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	GLTFModel* self = new GLTFModel();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
}

