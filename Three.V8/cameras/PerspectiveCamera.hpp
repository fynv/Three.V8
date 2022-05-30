#pragma once

#include "WrapperUtils.hpp"
#include "cameras/Camera.hpp"
#include <cameras/PerspectiveCamera.h>

class WrapperPerspectiveCamera
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetIsPerspectiveCamera(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

	static void GetFov(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetFov(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetAspect(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetAspect(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetNear(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetNear(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetFar(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetFar(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void UpdateProjectionMatrix(const v8::FunctionCallbackInfo<v8::Value>& info);

};


v8::Local<v8::FunctionTemplate> WrapperPerspectiveCamera::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperCamera::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "isPerspectiveCamera").ToLocalChecked(), GetIsPerspectiveCamera, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "fov").ToLocalChecked(), GetFov, SetFov);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "aspect").ToLocalChecked(), GetAspect, SetAspect);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "near").ToLocalChecked(), GetNear, SetNear);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "far").ToLocalChecked(), GetFar, SetFar);
	templ->InstanceTemplate()->Set(isolate, "updateProjectionMatrix", v8::FunctionTemplate::New(isolate, UpdateProjectionMatrix));
	return templ;
}

void WrapperPerspectiveCamera::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	float fov = 50.0f;
	float aspect = 1.0f;
	float z_near = 0.1f;
	float z_far = 200.0f;

	if (info.Length() > 0)
	{
		fov = (float)info[0].As<v8::Number>()->Value();
		if (info.Length() > 1)
		{
			aspect = (float)info[1].As<v8::Number>()->Value();
			if (info.Length() > 2)
			{
				z_near = (float)info[2].As<v8::Number>()->Value();
				if (info.Length() > 3)
				{
					z_far = (float)info[3].As<v8::Number>()->Value();
				}
			}
		}
	}
	PerspectiveCamera* self = new PerspectiveCamera(fov, aspect, z_near, z_far);
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
	info.This()->SetInternalField(1, v8::External::New(info.GetIsolate(), WrapperObject3D::dtor));
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This());
}

void WrapperPerspectiveCamera::GetIsPerspectiveCamera(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Local<v8::Boolean> ret = v8::Boolean::New(info.GetIsolate(), true);
	info.GetReturnValue().Set(ret);
}

void WrapperPerspectiveCamera::GetFov(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	PerspectiveCamera* self = get_self<PerspectiveCamera>(info);
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)self->fov);
	info.GetReturnValue().Set(ret);
}

void WrapperPerspectiveCamera::SetFov(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	PerspectiveCamera* self = get_self<PerspectiveCamera>(info);	
	self->fov = (float)value.As<v8::Number>()->Value();
}

void WrapperPerspectiveCamera::GetAspect(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	PerspectiveCamera* self = get_self<PerspectiveCamera>(info);
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)self->aspect);
	info.GetReturnValue().Set(ret);
}

void WrapperPerspectiveCamera::SetAspect(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	PerspectiveCamera* self = get_self<PerspectiveCamera>(info);
	self->aspect = (float)value.As<v8::Number>()->Value();
}

void WrapperPerspectiveCamera::GetNear(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	PerspectiveCamera* self = get_self<PerspectiveCamera>(info);
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)self->z_near);
	info.GetReturnValue().Set(ret);
}

void WrapperPerspectiveCamera::SetNear(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	PerspectiveCamera* self = get_self<PerspectiveCamera>(info);
	self->z_near = (float)value.As<v8::Number>()->Value();
}

void WrapperPerspectiveCamera::GetFar(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	PerspectiveCamera* self = get_self<PerspectiveCamera>(info);
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)self->z_far);
	info.GetReturnValue().Set(ret);
}

void WrapperPerspectiveCamera::SetFar(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	PerspectiveCamera* self = get_self<PerspectiveCamera>(info);
	self->z_far = (float)value.As<v8::Number>()->Value();
}

void WrapperPerspectiveCamera::UpdateProjectionMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	PerspectiveCamera* self = get_self<PerspectiveCamera>(info);
	self->updateProjectionMatrix();
}
