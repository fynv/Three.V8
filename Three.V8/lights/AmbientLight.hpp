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
	LocalContext lctx(info);
	AmbientLight* self = new AmbientLight();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), WrapperIndirectLight::dtor);
}

void WrapperAmbientLight::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AmbientLight* self = lctx.self<AmbientLight>();	
	v8::Local<v8::Object> color = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->color, color);
	info.GetReturnValue().Set(color);
}

void WrapperAmbientLight::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AmbientLight* self = lctx.self<AmbientLight>();
	lctx.vec3_to_jvec3(self->color, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperAmbientLight::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AmbientLight* self = lctx.self<AmbientLight>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->color.x);
		lctx.jnum_to_num(info[1], self->color.y);
		lctx.jnum_to_num(info[2], self->color.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->color);
	}
}

void WrapperAmbientLight::GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AmbientLight* self = lctx.self<AmbientLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->intensity));
}

void WrapperAmbientLight::SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	AmbientLight* self = lctx.self<AmbientLight>();
	lctx.jnum_to_num(value, self->intensity);
}

