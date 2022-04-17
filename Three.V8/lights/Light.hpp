#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <lights/Light.h>

class WrapperLight
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor);	

private:
	static void GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};


v8::Local<v8::FunctionTemplate> WrapperLight::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "color").ToLocalChecked(), GetColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getColor", v8::FunctionTemplate::New(isolate, GetColor));
	templ->InstanceTemplate()->Set(isolate, "setColor", v8::FunctionTemplate::New(isolate, SetColor));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "intensity").ToLocalChecked(), GetIntensity, SetIntensity);
	return templ;
}


void WrapperLight::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	Light* self = get_self<Light>(info);
	v8::Local<v8::Object> color = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->color, color);
	info.GetReturnValue().Set(color);
}

void WrapperLight::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Light* self = get_self<Light>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->color, out);
	info.GetReturnValue().Set(out);
}

void WrapperLight::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Light* self = get_self<Light>(info);
	if (info[0]->IsNumber())
	{
		self->color.x = (float)info[0].As<v8::Number>()->Value();
		self->color.y = (float)info[1].As<v8::Number>()->Value();
		self->color.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, self->color);
	}
}

void WrapperLight::GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Light* self = get_self<Light>(info);
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)self->intensity);
	info.GetReturnValue().Set(ret);
}

void WrapperLight::SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Light* self = get_self<Light>(info);
	self->intensity = (float)value.As<v8::Number>()->Value();
}

