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
	LocalContext lctx(info);
	v8::Local<v8::Value> block = lctx.get_property(lctx.holder, "_block");
	info.GetReturnValue().Set(block);
}

void WrapperUIElement::SetBlock(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UIElement* self = lctx.self<UIElement>();
	UIBlock* block = lctx.jobj_to_obj<UIBlock>(value);
	self->block = block;
	lctx.set_property(lctx.holder, "_block", value);	
}

void WrapperUIElement::GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIElement* self = lctx.self<UIElement>();
	v8::Local<v8::Object> origin = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->origin, origin);
	info.GetReturnValue().Set(origin);
}


void WrapperUIElement::GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIElement* self = lctx.self<UIElement>();
	lctx.vec2_to_jvec2(self->origin, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUIElement::SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIElement* self = lctx.self<UIElement>();
	glm::vec2 origin;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], origin.x);
		lctx.jnum_to_num(info[1], origin.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], origin);
	}

	if (self->origin != origin)
	{
		self->origin = origin;
		self->appearance_changed = true;
	}
}
