#pragma once

#include "WrapperUtils.hpp"
#include <lights/IndirectLight.h>


class WrapperIndirectLight
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void GetDiffuseThresh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetDiffuseThresh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetDiffuseHigh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetDiffuseHigh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetDiffuseLow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetDiffuseLow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetSpecularThresh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetSpecularThresh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetSpecularHigh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetSpecularHigh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetSpecularLow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetSpecularLow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetDynamicMap(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetDynamicMap(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};

v8::Local<v8::FunctionTemplate> WrapperIndirectLight::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "diffuseThresh").ToLocalChecked(), GetDiffuseThresh, SetDiffuseThresh);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "diffuseHigh").ToLocalChecked(), GetDiffuseHigh, SetDiffuseHigh);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "diffuseLow").ToLocalChecked(), GetDiffuseLow, SetDiffuseLow);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "specularThresh").ToLocalChecked(), GetSpecularThresh, SetSpecularThresh);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "specularHigh").ToLocalChecked(), GetSpecularHigh, SetSpecularHigh);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "specularLow").ToLocalChecked(), GetSpecularLow, SetSpecularLow);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "dynamicMap").ToLocalChecked(), GetDynamicMap, SetDynamicMap);
	return templ;
}


void WrapperIndirectLight::dtor(void* ptr, GameContext* ctx)
{
	delete (IndirectLight*)ptr;
}

void WrapperIndirectLight::GetDiffuseThresh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->diffuse_thresh));
}

void WrapperIndirectLight::SetDiffuseThresh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	lctx.jnum_to_num(value, self->diffuse_thresh);
}

void WrapperIndirectLight::GetDiffuseHigh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->diffuse_high));
}

void WrapperIndirectLight::SetDiffuseHigh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	lctx.jnum_to_num(value, self->diffuse_high);
}

void WrapperIndirectLight::GetDiffuseLow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->diffuse_low));
}

void WrapperIndirectLight::SetDiffuseLow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	lctx.jnum_to_num(value, self->diffuse_low);
}

void WrapperIndirectLight::GetSpecularThresh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->specular_thresh));
}

void WrapperIndirectLight::SetSpecularThresh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->specular_thresh));
}

void WrapperIndirectLight::GetSpecularHigh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->specular_high));
}

void WrapperIndirectLight::SetSpecularHigh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->specular_high));
}

void WrapperIndirectLight::GetSpecularLow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->specular_low));
}

void WrapperIndirectLight::SetSpecularLow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->specular_low));
}

void WrapperIndirectLight::GetDynamicMap(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->dynamic_map));
}

void WrapperIndirectLight::SetDynamicMap(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	IndirectLight* self = lctx.self<IndirectLight>();
	self->set_dynamic_map(value.As<v8::Boolean>()->Value());
}
