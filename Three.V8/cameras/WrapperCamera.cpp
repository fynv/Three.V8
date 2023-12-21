#include "WrapperUtils.hpp"
#include "core/WrapperObject3D.h"
#include <cameras/Camera.h>

#include "WrapperCamera.h"

void WrapperCamera::define(ClassDefinition& cls)
{
	WrapperObject3D::define(cls);
	cls.name = "Camera";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "matrixWorldInverse",  GetMatrixWorldInverse },
		{ "projectionMatrix",  GetProjectionMatrix },
		{ "projectionMatrixInverse",  GetProjectionMatrixInverse },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getMatrixWorldInverse", GetMatrixWorldInverse },
		{"getProjectionMatrix", GetProjectionMatrix },
		{"getProjectionMatrixInverse", GetProjectionMatrixInverse },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}


void* WrapperCamera::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new Camera;
}

void WrapperCamera::GetMatrixWorldInverse(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Camera* self = lctx.self<Camera>();
	v8::Local<v8::Object> matrix = v8::Object::New(lctx.isolate);
	lctx.mat4_to_jmat4(self->matrixWorldInverse, matrix);
	info.GetReturnValue().Set(matrix);
}


void WrapperCamera::GetMatrixWorldInverse(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Camera* self = lctx.self<Camera>();
	lctx.mat4_to_jmat4(self->matrixWorldInverse, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperCamera::GetProjectionMatrix(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Camera* self = lctx.self<Camera>();
	v8::Local<v8::Object> matrix = v8::Object::New(lctx.isolate);
	lctx.mat4_to_jmat4(self->projectionMatrix, matrix);
	info.GetReturnValue().Set(matrix);
}


void WrapperCamera::GetProjectionMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Camera* self = lctx.self<Camera>();
	lctx.mat4_to_jmat4(self->projectionMatrix, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperCamera::GetProjectionMatrixInverse(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Camera* self = lctx.self<Camera>();
	v8::Local<v8::Object> matrix = v8::Object::New(lctx.isolate);
	lctx.mat4_to_jmat4(self->projectionMatrixInverse, matrix);
	info.GetReturnValue().Set(matrix);
}


void WrapperCamera::GetProjectionMatrixInverse(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Camera* self = lctx.self<Camera>();
	lctx.mat4_to_jmat4(self->projectionMatrixInverse, info[0]);
	info.GetReturnValue().Set(info[0]);
}

