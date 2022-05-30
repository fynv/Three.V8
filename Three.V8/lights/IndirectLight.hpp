#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <lights/IndirectLight.h>


class WrapperIndirectLight
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor);
	static void dtor(void* ptr);

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
	return templ;
}


void WrapperIndirectLight::dtor(void* ptr)
{
	delete (IndirectLight*)ptr;
}

void WrapperIndirectLight::GetDiffuseThresh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);	
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->diffuse_thresh));
}

void WrapperIndirectLight::SetDiffuseThresh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	self->diffuse_thresh = (float)value.As<v8::Number>()->Value();
}

void WrapperIndirectLight::GetDiffuseHigh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->diffuse_high));
}

void WrapperIndirectLight::SetDiffuseHigh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	self->diffuse_high = (float)value.As<v8::Number>()->Value();
}

void WrapperIndirectLight::GetDiffuseLow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->diffuse_low));
}

void WrapperIndirectLight::SetDiffuseLow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	self->diffuse_low = (float)value.As<v8::Number>()->Value();
}

void WrapperIndirectLight::GetSpecularThresh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->specular_thresh));
}

void WrapperIndirectLight::SetSpecularThresh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	self->specular_thresh = (float)value.As<v8::Number>()->Value();
}

void WrapperIndirectLight::GetSpecularHigh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->specular_high));
}

void WrapperIndirectLight::SetSpecularHigh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	self->specular_high = (float)value.As<v8::Number>()->Value();
}

void WrapperIndirectLight::GetSpecularLow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->specular_low));
}

void WrapperIndirectLight::SetSpecularLow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	IndirectLight* self = get_self<IndirectLight>(info);
	self->specular_low = (float)value.As<v8::Number>()->Value();
}
