#include "WrapperUtils.hpp"
#include "WrapperIndirectLight.h"
#include <lights/AmbientLight.h>

#include "WrapperAmbientLight.h"

void WrapperAmbientLight::define(ClassDefinition& cls)
{
	WrapperIndirectLight::define(cls);
	cls.name = "AmbientLight";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "color",  GetColor },
		{ "intensity",  GetIntensity, SetIntensity },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getColor", GetColor },
		{"setColor", SetColor },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());

}

void* WrapperAmbientLight::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new AmbientLight;
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

