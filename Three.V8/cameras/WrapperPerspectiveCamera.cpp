#include "WrapperUtils.hpp"
#include "WrapperCamera.h"
#include <cameras/PerspectiveCamera.h>

#include "WrapperPerspectiveCamera.h"

void WrapperPerspectiveCamera::define(ClassDefinition& cls)
{
	WrapperCamera::define(cls);

	cls.name = "PerspectiveCamera";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "isPerspectiveCamera",  GetIsPerspectiveCamera },
		{ "fov",  GetFov, SetFov },
		{ "aspect",  GetAspect, SetAspect },
		{ "near",  GetFar, SetNear },
		{ "far",  GetFar, SetFar },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"updateProjectionMatrix", UpdateProjectionMatrix },		
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());

}

void* WrapperPerspectiveCamera::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
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
	return new PerspectiveCamera(fov, aspect, z_near, z_far);
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

