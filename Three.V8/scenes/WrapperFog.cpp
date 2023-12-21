#include "WrapperUtils.hpp"
#include "scenes/Fog.h"
#include "WrapperFog.h"

void WrapperFog::define(ClassDefinition& cls)
{
	cls.name = "Fog";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "color",  GetColor },
		{ "density",  GetDensity, SetDensity },
		{ "maxNumSteps",  GetMaxNumSteps, SetMaxNumSteps },
		{ "minStep",  GetMinStep, SetMinStep },
	};
	cls.methods = {
		{"getColor", GetColor },
		{"setColor", SetColor },
	};
}

void* WrapperFog::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new Fog;
}

void WrapperFog::dtor(void* ptr, GameContext* ctx)
{
	delete (Fog*)ptr;
}

void WrapperFog::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Fog* self = lctx.self<Fog>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->color, position);
	info.GetReturnValue().Set(position);
}

void WrapperFog::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Fog* self = lctx.self<Fog>();
	lctx.vec3_to_jvec3(self->color, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperFog::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Fog* self = lctx.self<Fog>();
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

void WrapperFog::GetDensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Fog* self = lctx.self<Fog>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->density));
}

void WrapperFog::SetDensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Fog* self = lctx.self<Fog>();
	lctx.jnum_to_num(value, self->density);
}


void WrapperFog::GetMaxNumSteps(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Fog* self = lctx.self<Fog>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->max_num_steps));
}

void WrapperFog::SetMaxNumSteps(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Fog* self = lctx.self<Fog>();
	lctx.jnum_to_num(value, self->max_num_steps);
}


void WrapperFog::GetMinStep(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Fog* self = lctx.self<Fog>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->min_step));
}

void WrapperFog::SetMinStep(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Fog* self = lctx.self<Fog>();
	lctx.jnum_to_num(value, self->min_step);
}

