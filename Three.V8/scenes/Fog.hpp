#pragma once

#include "WrapperUtils.hpp"
#include "scenes/Fog.h"

class WrapperFog
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);	

private:
	static void GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetDensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetDensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetMaxNumSteps(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetMaxNumSteps(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetMinStep(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetMinStep(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};


v8::Local<v8::FunctionTemplate> WrapperFog::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "color").ToLocalChecked(), GetColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getColor", v8::FunctionTemplate::New(isolate, GetColor));
	templ->InstanceTemplate()->Set(isolate, "setColor", v8::FunctionTemplate::New(isolate, SetColor));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "density").ToLocalChecked(), GetDensity, SetDensity);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "maxNumSteps").ToLocalChecked(), GetMaxNumSteps, SetMaxNumSteps);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "minStep").ToLocalChecked(), GetMinStep, SetMinStep);

	return templ;
}

void WrapperFog::dtor(void* ptr, GameContext* ctx)
{
	delete (Fog*)ptr;
}


void WrapperFog::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Fog* self = new Fog();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), WrapperFog::dtor);
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
