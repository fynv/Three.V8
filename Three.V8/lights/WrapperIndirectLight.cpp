#include "WrapperUtils.hpp"
#include <lights/IndirectLight.h>
#include "WrapperIndirectLight.h"

void WrapperIndirectLight::define(ClassDefinition& cls)
{
	cls.name = "IndirectLight";	
	cls.dtor = dtor;
	cls.properties = {
		{ "diffuseThresh",  GetDiffuseThresh, SetDiffuseThresh },
		{ "diffuseHigh",  GetDiffuseHigh, SetDiffuseHigh },
		{ "diffuseLow", GetDiffuseLow, SetDiffuseLow },
		{ "specularThresh",  GetSpecularThresh, SetSpecularThresh },
		{ "specularHigh", GetSpecularHigh, SetSpecularHigh },
		{ "specularLow", GetSpecularLow, SetSpecularLow },
		{ "dynamicMap", GetDynamicMap, SetDynamicMap },
	};
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
