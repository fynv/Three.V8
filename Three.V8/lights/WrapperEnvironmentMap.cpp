#include "WrapperUtils.hpp"
#include "WrapperIndirectLight.h"
#include <lights/EnvironmentMap.h>
#include "WrapperEnvironmentMap.h"

void WrapperEnvironmentMap::define(ClassDefinition& cls)
{
	WrapperIndirectLight::define(cls);
	cls.name = "EnvironmentMap";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "shCoefficients", GetSHCoefficients, SetSHCoefficients },		
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

}

void* WrapperEnvironmentMap::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new EnvironmentMap;
}

void WrapperEnvironmentMap::GetSHCoefficients(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	EnvironmentMap* self = lctx.self<EnvironmentMap>();
	v8::Local<v8::Array> ret = v8::Array::New(lctx.isolate, 9);
	for (int i = 0; i < 9; i++)
	{
		glm::vec3 col = self->shCoefficients[i];
		v8::Local<v8::Array> coef = v8::Array::New(lctx.isolate, 3);
		coef->Set(lctx.context, 0, lctx.num_to_jnum(col.x));
		coef->Set(lctx.context, 1, lctx.num_to_jnum(col.y));
		coef->Set(lctx.context, 2, lctx.num_to_jnum(col.z));
		ret->Set(lctx.context, i, coef);
	}
	info.GetReturnValue().Set(ret);
}

void WrapperEnvironmentMap::SetSHCoefficients(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	EnvironmentMap* self = lctx.self<EnvironmentMap>();
	v8::Local<v8::Array> arr = value.As<v8::Array>();
	for (int i = 0; i < 9; i++)
	{
		glm::vec4& col = self->shCoefficients[i];
		v8::Local<v8::Array> coef = arr->Get(lctx.context, i).ToLocalChecked().As<v8::Array>();
		lctx.jnum_to_num(coef->Get(lctx.context, 0).ToLocalChecked(), col.x);
		lctx.jnum_to_num(coef->Get(lctx.context, 1).ToLocalChecked(), col.y);
		lctx.jnum_to_num(coef->Get(lctx.context, 2).ToLocalChecked(), col.z);
	}
}

