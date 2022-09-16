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
	LocalContext lctx(info);
	HemisphereLight* self = new HemisphereLight();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), WrapperIndirectLight::dtor);
}

void WrapperHemisphereLight::GetSkyColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereLight* self = lctx.self<HemisphereLight>();
	v8::Local<v8::Object> color = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->skyColor, color);
	info.GetReturnValue().Set(color);
}

void WrapperHemisphereLight::GetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereLight* self = lctx.self<HemisphereLight>();
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	lctx.vec3_to_jvec3(self->skyColor, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperHemisphereLight::SetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereLight* self = lctx.self<HemisphereLight>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->skyColor.x);
		lctx.jnum_to_num(info[1], self->skyColor.y);
		lctx.jnum_to_num(info[2], self->skyColor.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->skyColor);
	}
}


void WrapperHemisphereLight::GetGroundColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereLight* self = lctx.self<HemisphereLight>();
	v8::Local<v8::Object> color = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->groundColor, color);
	info.GetReturnValue().Set(color);
}

void WrapperHemisphereLight::GetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereLight* self = lctx.self<HemisphereLight>();
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	lctx.vec3_to_jvec3(self->groundColor, info[0]);
	info.GetReturnValue().Set(info[0]);

}

void WrapperHemisphereLight::SetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereLight* self = lctx.self<HemisphereLight>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->groundColor.x);
		lctx.jnum_to_num(info[1], self->groundColor.y);
		lctx.jnum_to_num(info[2], self->groundColor.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->groundColor);
	}
}

void WrapperHemisphereLight::GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereLight* self = lctx.self<HemisphereLight>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->intensity));
}

void WrapperHemisphereLight::SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	HemisphereLight* self = lctx.self<HemisphereLight>();
	lctx.jnum_to_num(value, self->intensity);
}

