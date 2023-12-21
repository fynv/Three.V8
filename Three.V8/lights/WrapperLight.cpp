#include "WrapperUtils.hpp"
#include "core/WrapperObject3D.h"
#include <lights/Light.h>
#include "WrapperLight.h"


void WrapperLight::define(ClassDefinition& cls)
{
	WrapperObject3D::define(cls);
	cls.name = "Light";

	std::vector<AccessorDefinition> props = {
		{ "color",  GetColor },
		{ "intensity", GetIntensity, SetIntensity },
		{ "diffuseThresh",  GetDiffuseThresh, SetDiffuseThresh },
		{ "diffuseHigh",  GetDiffuseHigh, SetDiffuseHigh },
		{ "diffuseLow", GetDiffuseLow, SetDiffuseLow },
		{ "specularThresh",  GetSpecularThresh, SetSpecularThresh },
		{ "specularHigh", GetSpecularHigh, SetSpecularHigh },
		{ "specularLow", GetSpecularLow, SetSpecularLow },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getColor", GetColor },
		{"setColor", SetColor },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}


void WrapperLight::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	v8::Local<v8::Object> color = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->color, color);
	info.GetReturnValue().Set(color);
}

void WrapperLight::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	lctx.vec3_to_jvec3(self->color, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperLight::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
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

void WrapperLight::GetIntensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->intensity));
}

void WrapperLight::SetIntensity(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	lctx.jnum_to_num(value, self->intensity);
}


void WrapperLight::GetDiffuseThresh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->diffuse_thresh));
}

void WrapperLight::SetDiffuseThresh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	lctx.jnum_to_num(value, self->diffuse_thresh);
}

void WrapperLight::GetDiffuseHigh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->diffuse_high));
}

void WrapperLight::SetDiffuseHigh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	lctx.jnum_to_num(value, self->diffuse_high);
}

void WrapperLight::GetDiffuseLow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->diffuse_low));
}

void WrapperLight::SetDiffuseLow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	lctx.jnum_to_num(value, self->diffuse_low);
}

void WrapperLight::GetSpecularThresh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->specular_thresh));
}

void WrapperLight::SetSpecularThresh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	lctx.jnum_to_num(value, self->specular_thresh);
}

void WrapperLight::GetSpecularHigh(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->specular_high));
}

void WrapperLight::SetSpecularHigh(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	lctx.jnum_to_num(value, self->specular_high);
}

void WrapperLight::GetSpecularLow(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->specular_low));
}

void WrapperLight::SetSpecularLow(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Light* self = lctx.self<Light>();
	lctx.jnum_to_num(value, self->specular_low);
}

