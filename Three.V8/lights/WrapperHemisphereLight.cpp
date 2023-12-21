#include "WrapperUtils.hpp"
#include "WrapperIndirectLight.h"
#include <lights/HemisphereLight.h>

#include "WrapperHemisphereLight.h"


void WrapperHemisphereLight::define(ClassDefinition& cls)
{
	WrapperIndirectLight::define(cls);
	cls.name = "HemisphereLight";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "skyColor",  GetSkyColor },
		{ "groundColor",  GetGroundColor },
		{ "intensity",  GetIntensity, SetIntensity },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getSkyColor", GetSkyColor },
		{"setSkyColor", SetSkyColor },
		{"getGroundColor", GetGroundColor },
		{"setGroundColor", SetGroundColor },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());

}

void* WrapperHemisphereLight::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new HemisphereLight;
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

