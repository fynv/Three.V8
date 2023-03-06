#pragma once

#include "WrapperUtils.hpp"
#include "lights/Light.hpp"
#include <lights/DirectionalLight.h>
#include <lights/DirectionalLightShadow.h>

class WrapperDirectionalLight
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetTarget(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetTarget(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
	static void SetShadow(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetShadowProjection(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetShadowRadius(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetBias(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetBias(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};


v8::Local<v8::FunctionTemplate> WrapperDirectionalLight::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperLight::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "target").ToLocalChecked(), GetTarget, SetTarget);
	templ->InstanceTemplate()->Set(isolate, "setShadow", v8::FunctionTemplate::New(isolate, SetShadow));
	templ->InstanceTemplate()->Set(isolate, "setShadowProjection", v8::FunctionTemplate::New(isolate, SetShadowProjection));
	templ->InstanceTemplate()->Set(isolate, "setShadowRadius", v8::FunctionTemplate::New(isolate, SetShadowRadius));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "bias").ToLocalChecked(), GetBias, SetBias);
	return templ;
}

void WrapperDirectionalLight::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = new DirectionalLight();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), WrapperObject3D::dtor);
}


void WrapperDirectionalLight::GetTarget(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> target = v8::Null(lctx.isolate);
	if (lctx.has_property(holder, "_target"))
	{
		target = lctx.get_property(holder, "_target");
	}
	info.GetReturnValue().Set(target);
}

void WrapperDirectionalLight::SetTarget(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();
	if (value->IsNull())
	{
		self->target = nullptr;
	}
	else
	{
		Object3D* target = lctx.jobj_to_obj<Object3D>(value);
		self->target = target;
	}
	
	v8::Local<v8::Object> holder = info.Holder();
	lctx.set_property(holder, "_target", value);	
}

void WrapperDirectionalLight::SetShadow(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();

	bool enable = info[0].As<v8::Boolean>()->Value();
	int width = -1;
	int height = -1;
	if (info.Length() > 2)
	{
		lctx.jnum_to_num(info[1], width);
		lctx.jnum_to_num(info[2], height);
	}
	self->setShadow(enable, width, height);
}

void  WrapperDirectionalLight::SetShadowProjection(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();

	float left, right, bottom, top, zNear, zFar;
	lctx.jnum_to_num(info[0], left);
	lctx.jnum_to_num(info[1], right);
	lctx.jnum_to_num(info[2], bottom);
	lctx.jnum_to_num(info[3], top);
	lctx.jnum_to_num(info[4], zNear);
	lctx.jnum_to_num(info[5], zFar);
	self->setShadowProjection(left, right, bottom, top, zNear, zFar);
}

void  WrapperDirectionalLight::SetShadowRadius(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();

	float radius;
	lctx.jnum_to_num(info[0], radius);	
	self->SetShadowRadius(radius);
}


void WrapperDirectionalLight::GetBias(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();
	float bias = 0.001f;
	if (self->shadow != nullptr)
	{
		bias = self->shadow->m_bias;
	}
	info.GetReturnValue().Set(lctx.num_to_jnum(bias));
}

void WrapperDirectionalLight::SetBias(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();
	if (self->shadow != nullptr)
	{
		lctx.jnum_to_num(value, self->shadow->m_bias);
	}
}
