#pragma once

#include "WrapperUtils.hpp"
#include <gui/UIElement.h>

class WrapperUIElement
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void GetBlock(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetBlock(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);

};


v8::Local<v8::FunctionTemplate> WrapperUIElement::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "block").ToLocalChecked(), GetBlock, SetBlock);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "origin").ToLocalChecked(), GetOrigin, 0);
	templ->InstanceTemplate()->Set(isolate, "getOrigin", v8::FunctionTemplate::New(isolate, GetOrigin));
	templ->InstanceTemplate()->Set(isolate, "setOrigin", v8::FunctionTemplate::New(isolate, SetOrigin));

	return templ;
}


void WrapperUIElement::dtor(void* ptr, GameContext* ctx)
{
	delete (UIElement*)ptr;
}


void WrapperUIElement::GetBlock(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> block = v8::Null(isolate);
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_block").ToLocalChecked()).ToChecked())
	{
		block = holder->Get(context, v8::String::NewFromUtf8(isolate, "_block").ToLocalChecked()).ToLocalChecked();
	}
	info.GetReturnValue().Set(block);
}

void WrapperUIElement::SetBlock(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIElement* self = (UIElement*)holder->GetAlignedPointerFromInternalField(0);
	holder->Set(context, v8::String::NewFromUtf8(isolate, "_block").ToLocalChecked(), value);
	UIBlock* block = (UIBlock*)value.As<v8::Object>()->GetAlignedPointerFromInternalField(0);
	self->block = block;
}

void WrapperUIElement::GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UIElement* self = get_self<UIElement>(info);
	v8::Local<v8::Object> origin = v8::Object::New(isolate);
	vec2_to_jvec2(isolate, self->origin, origin);
	info.GetReturnValue().Set(origin);
}


void WrapperUIElement::GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIElement* self = get_self<UIElement>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec2_to_jvec2(isolate, self->origin, out);
	info.GetReturnValue().Set(out);
}


void WrapperUIElement::SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIElement* self = get_self<UIElement>(info);
	glm::vec2 origin;
	if (info[0]->IsNumber())
	{
		origin.x = (float)info[0].As<v8::Number>()->Value();
		origin.y = (float)info[1].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec2_to_vec2(isolate, in, origin);
	}
	if (self->origin != origin)
	{
		self->origin = origin;
		self->appearance_changed = true;
	}
}