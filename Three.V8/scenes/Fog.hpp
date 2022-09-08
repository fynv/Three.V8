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
	Fog* self = new Fog();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), WrapperFog::dtor);
}

void WrapperFog::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	Fog* self = get_self<Fog>(info);
	v8::Local<v8::Object> position = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->color, position);
	info.GetReturnValue().Set(position);
}

void WrapperFog::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	Fog* self = get_self<Fog>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->color, out);
	info.GetReturnValue().Set(out);
}

void WrapperFog::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	Fog* self = get_self<Fog>(info);
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

void WrapperFog::GetDensity(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Fog* self = get_self<Fog>(info);
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->density));
}

void WrapperFog::SetDensity(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Fog* self = get_self<Fog>(info);
	self->density = (float)value.As<v8::Number>()->Value();
}


void WrapperFog::GetMaxNumSteps(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Fog* self = get_self<Fog>(info);
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->max_num_steps));
}

void WrapperFog::SetMaxNumSteps(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Fog* self = get_self<Fog>(info);
	self->max_num_steps = (int)value.As<v8::Number>()->Value();
}


void WrapperFog::GetMinStep(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Fog* self = get_self<Fog>(info);
	info.GetReturnValue().Set(v8::Number::New(info.GetIsolate(), (double)self->min_step));
}

void WrapperFog::SetMinStep(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Fog* self = get_self<Fog>(info);
	self->min_step = (float)value.As<v8::Number>()->Value();
}
