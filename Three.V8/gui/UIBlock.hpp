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
	LocalContext lctx(info);
	UIBlock* self = new UIBlock();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), WrapperUIElement::dtor);
}

void WrapperUIBlock::GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIBlock* self = lctx.self<UIBlock>();
	v8::Local<v8::Object> size = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->size, size);
	info.GetReturnValue().Set(size);
}


void WrapperUIBlock::GetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIBlock* self = lctx.self<UIBlock>();
	lctx.vec2_to_jvec2(self->size, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUIBlock::SetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIBlock* self = lctx.self<UIBlock>();
	glm::vec2 size;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], size.x);
		lctx.jnum_to_num(info[1], size.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], size);
	}
	if (self->size != size)
	{
		self->size = size;
		self->appearance_changed = true;
	}
}


void WrapperUIBlock::GetScissorEnabled(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIBlock* self = lctx.self<UIBlock>();
	v8::Local<v8::Boolean> ret = v8::Boolean::New(lctx.isolate, self->scissor_enabled);
	info.GetReturnValue().Set(ret);
}

void WrapperUIBlock::SetScissorEnabled(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UIBlock* self = lctx.self<UIBlock>();
	self->scissor_enabled = value.As<v8::Boolean>()->Value();
}
