#pragma once

#include "WrapperUtils.hpp"
#include "IndirectLight.hpp"
#include <lights/AmbientLight.h>


class WrapperAmbientLight
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};

v8::Local<v8::FunctionTemplate> WrapperAmbientLight::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperIndirectLight::create_template(isolate, constructor);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "color").ToLocalChecked(), GetColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getColor", v8::FunctionTemplate::New(isolate, GetColor));
	templ->InstanceTemplate()->Set(isolate, "setColor", v8::FunctionTemplate::New(isolate, SetColor));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "intensity").ToLocalChecked(), GetIntensity, SetIntensity);

	return templ;
}

void WrapperAmbientLight::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	AmbientLight* self = new AmbientLight();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), WrapperIndirectLight::dtor);
}

void WrapperAmbientLight::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	AmbientLight* self = get_self<AmbientLight>(info);
	v8::Local<v8::Object> position = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->color, position);
	info.GetReturnValue().Set(position);
}

void WrapperAmbientLight::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	AmbientLight* self = get_self<AmbientLight>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->color, out);
	info.GetReturnValue().Set(out);
}

void WrapperAmbientLight::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	AmbientLight* self = get_self<AmbientLight>(info);
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

void WrapperAmbientLight::GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	AmbientLight* self = get_self<AmbientLight>(info);	
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->intensity));
}

void WrapperAmbientLight::SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	AmbientLight* self = get_self<AmbientLight>(info);	
	self->intensity = (float)value.As<v8::Number>()->Value();
}

