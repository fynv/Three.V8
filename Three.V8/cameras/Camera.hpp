#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <cameras/Camera.h>

class WrapperCamera
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetMatrixWorldInverse(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetMatrixWorldInverse(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetProjectionMatrix(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetProjectionMatrix(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetProjectionMatrixInverse(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetProjectionMatrixInverse(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperCamera::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "matrixWorldInverse").ToLocalChecked(), GetMatrixWorldInverse, 0);
	templ->InstanceTemplate()->Set(isolate, "getMatrixWorldInverse", v8::FunctionTemplate::New(isolate, GetMatrixWorldInverse));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "projectionMatrix").ToLocalChecked(), GetProjectionMatrix, 0);
	templ->InstanceTemplate()->Set(isolate, "getProjectionMatrix", v8::FunctionTemplate::New(isolate, GetProjectionMatrix));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "projectionMatrixInverse").ToLocalChecked(), GetProjectionMatrixInverse, 0);
	templ->InstanceTemplate()->Set(isolate, "getProjectionMatrixInverse", v8::FunctionTemplate::New(isolate, GetProjectionMatrixInverse));
	return templ;
}

void WrapperCamera::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	Camera* self = new Camera();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
	WrapperObject3D::Init(info.GetIsolate(), info.This());
}


void WrapperCamera::GetMatrixWorldInverse(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Camera* self = get_self<Camera>(info);
	v8::Local<v8::Object> matrix = v8::Object::New(isolate);
	mat4_to_jmat4(isolate, self->matrixWorldInverse, matrix);
	info.GetReturnValue().Set(matrix);
}


void WrapperCamera::GetMatrixWorldInverse(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Camera* self = get_self<Camera>(info);
	v8::Local<v8::Object> matrix = info[0].As<v8::Object>();
	mat4_to_jmat4(isolate, self->matrixWorldInverse, matrix);
	info.GetReturnValue().Set(matrix);
}

void WrapperCamera::GetProjectionMatrix(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Camera* self = get_self<Camera>(info);
	v8::Local<v8::Object> matrix = v8::Object::New(isolate);
	mat4_to_jmat4(isolate, self->projectionMatrix, matrix);
	info.GetReturnValue().Set(matrix);
}


void WrapperCamera::GetProjectionMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Camera* self = get_self<Camera>(info);
	v8::Local<v8::Object> matrix = info[0].As<v8::Object>();
	mat4_to_jmat4(isolate, self->projectionMatrix, matrix);
	info.GetReturnValue().Set(matrix);
}

void WrapperCamera::GetProjectionMatrixInverse(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Camera* self = get_self<Camera>(info);
	v8::Local<v8::Object> matrix = v8::Object::New(isolate);
	mat4_to_jmat4(isolate, self->projectionMatrixInverse, matrix);
	info.GetReturnValue().Set(matrix);
}


void WrapperCamera::GetProjectionMatrixInverse(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Camera* self = get_self<Camera>(info);
	v8::Local<v8::Object> matrix = info[0].As<v8::Object>();
	mat4_to_jmat4(isolate, self->projectionMatrixInverse, matrix);
	info.GetReturnValue().Set(matrix);
}
