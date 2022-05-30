#pragma once

#include "WrapperUtils.hpp"
#include "IndirectLight.hpp"
#include <lights/HemisphereLight.h>

class WrapperHemisphereLight
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetSkyColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetGroundColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};

v8::Local<v8::FunctionTemplate> WrapperHemisphereLight::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperIndirectLight::create_template(isolate, constructor);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "skyColor").ToLocalChecked(), GetSkyColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getSkyColor", v8::FunctionTemplate::New(isolate, GetSkyColor));
	templ->InstanceTemplate()->Set(isolate, "setSkyColor", v8::FunctionTemplate::New(isolate, SetSkyColor));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "groundColor").ToLocalChecked(), GetGroundColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getGroundColor", v8::FunctionTemplate::New(isolate, GetGroundColor));
	templ->InstanceTemplate()->Set(isolate, "setGroundColor", v8::FunctionTemplate::New(isolate, SetGroundColor));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "intensity").ToLocalChecked(), GetIntensity, SetIntensity);

	return templ;
}

void WrapperHemisphereLight::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	HemisphereLight* self = new HemisphereLight();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
	info.This()->SetInternalField(1, v8::External::New(info.GetIsolate(), WrapperIndirectLight::dtor));
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This());
}

void WrapperHemisphereLight::GetSkyColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereLight* self = get_self<HemisphereLight>(info);
	v8::Local<v8::Object> position = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->skyColor, position);
	info.GetReturnValue().Set(position);
}

void WrapperHemisphereLight::GetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereLight* self = get_self<HemisphereLight>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->skyColor, out);
	info.GetReturnValue().Set(out);
}

void WrapperHemisphereLight::SetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereLight* self = get_self<HemisphereLight>(info);
	if (info[0]->IsNumber())
	{
		self->skyColor.x = (float)info[0].As<v8::Number>()->Value();
		self->skyColor.y = (float)info[1].As<v8::Number>()->Value();
		self->skyColor.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, self->skyColor);
	}
}


void WrapperHemisphereLight::GetGroundColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereLight* self = get_self<HemisphereLight>(info);
	v8::Local<v8::Object> position = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->groundColor, position);
	info.GetReturnValue().Set(position);
}

void WrapperHemisphereLight::GetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereLight* self = get_self<HemisphereLight>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->groundColor, out);
	info.GetReturnValue().Set(out);
}

void WrapperHemisphereLight::SetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereLight* self = get_self<HemisphereLight>(info);
	if (info[0]->IsNumber())
	{
		self->groundColor.x = (float)info[0].As<v8::Number>()->Value();
		self->groundColor.y = (float)info[1].As<v8::Number>()->Value();
		self->groundColor.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, self->groundColor);
	}
}

void WrapperHemisphereLight::GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	HemisphereLight* self = get_self<HemisphereLight>(info);
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->intensity));
}

void WrapperHemisphereLight::SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	HemisphereLight* self = get_self<HemisphereLight>(info);
	self->intensity = (float)value.As<v8::Number>()->Value();
}

