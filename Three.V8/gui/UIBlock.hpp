#pragma once

#include "WrapperUtils.hpp"
#include "UIElement.hpp"
#include <gui/UIBlock.h>


class WrapperUIBlock
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetSize(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetScissorEnabled(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetScissorEnabled(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};


v8::Local<v8::FunctionTemplate> WrapperUIBlock::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperUIElement::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "size").ToLocalChecked(), GetSize, 0);
	templ->InstanceTemplate()->Set(isolate, "getSize", v8::FunctionTemplate::New(isolate, GetSize));
	templ->InstanceTemplate()->Set(isolate, "setSize", v8::FunctionTemplate::New(isolate, SetSize));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "scissorEnabled").ToLocalChecked(), GetScissorEnabled, SetScissorEnabled);
	return templ;
}

void WrapperUIBlock::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	UIBlock* self = new UIBlock();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), WrapperUIElement::dtor);
}

void WrapperUIBlock::GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UIBlock* self = get_self<UIBlock>(info);
	v8::Local<v8::Object> size = v8::Object::New(isolate);
	vec2_to_jvec2(isolate, self->size, size);
	info.GetReturnValue().Set(size);
}


void WrapperUIBlock::GetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIBlock* self = get_self<UIBlock>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec2_to_jvec2(isolate, self->size, out);
	info.GetReturnValue().Set(out);
}


void WrapperUIBlock::SetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIBlock* self = get_self<UIBlock>(info);
	glm::vec2 size;
	if (info[0]->IsNumber())
	{
		size.x = (float)info[0].As<v8::Number>()->Value();
		size.y = (float)info[1].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec2_to_vec2(isolate, in, size);
	}
	if (self->size != size)
	{
		self->size = size;
		self->appearance_changed = true;
	}
}


void WrapperUIBlock::GetScissorEnabled(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIBlock* self = get_self<UIBlock>(info);
	v8::Local<v8::Boolean> ret = v8::Boolean::New(isolate, self->scissor_enabled);
	info.GetReturnValue().Set(ret);
}

void WrapperUIBlock::SetScissorEnabled(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIBlock* self = get_self<UIBlock>(info);
	self->scissor_enabled = value.As<v8::Boolean>()->Value();
}
