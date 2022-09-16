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
	LocalContext lctx(info);

	float fov = 50.0f;
	float aspect = 1.0f;
	float z_near = 0.1f;
	float z_far = 200.0f;

	if (info.Length() > 0)
	{
		lctx.jnum_to_num(info[0], fov);		
		if (info.Length() > 1)
		{
			lctx.jnum_to_num(info[1], aspect);
			if (info.Length() > 2)
			{
				lctx.jnum_to_num(info[2], z_near);
				if (info.Length() > 3)
				{
					lctx.jnum_to_num(info[3], z_far);
				}
			}
		}
	}
	PerspectiveCamera* self = new PerspectiveCamera(fov, aspect, z_near, z_far);
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), WrapperObject3D::dtor);
}

void WrapperPerspectiveCamera::GetIsPerspectiveCamera(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Local<v8::Boolean> ret = v8::Boolean::New(info.GetIsolate(), true);
	info.GetReturnValue().Set(ret);
}

void WrapperPerspectiveCamera::GetFov(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	PerspectiveCamera* self = lctx.self<PerspectiveCamera>();	
	info.GetReturnValue().Set(lctx.num_to_jnum(self->fov));
}

void WrapperPerspectiveCamera::SetFov(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	PerspectiveCamera* self = lctx.self<PerspectiveCamera>();
	lctx.jnum_to_num(value, self->fov);
}

void WrapperPerspectiveCamera::GetAspect(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	PerspectiveCamera* self = lctx.self<PerspectiveCamera>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->aspect));	
}

void WrapperPerspectiveCamera::SetAspect(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	PerspectiveCamera* self = lctx.self<PerspectiveCamera>();
	lctx.jnum_to_num(value, self->aspect);
}

void WrapperPerspectiveCamera::GetNear(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	PerspectiveCamera* self = lctx.self<PerspectiveCamera>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->z_near));
}

void WrapperPerspectiveCamera::SetNear(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	PerspectiveCamera* self = lctx.self<PerspectiveCamera>();
	lctx.jnum_to_num(value, self->z_near);
}

void WrapperPerspectiveCamera::GetFar(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	PerspectiveCamera* self = lctx.self<PerspectiveCamera>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->z_far));
}

void WrapperPerspectiveCamera::SetFar(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	PerspectiveCamera* self = lctx.self<PerspectiveCamera>();
	lctx.jnum_to_num(value, self->z_far);
}

void WrapperPerspectiveCamera::UpdateProjectionMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	PerspectiveCamera* self = lctx.self<PerspectiveCamera>();
	self->updateProjectionMatrix();
}
