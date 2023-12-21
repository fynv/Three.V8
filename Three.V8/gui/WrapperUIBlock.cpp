#include "WrapperUtils.hpp"
#include "WrapperUIElement.h"
#include <gui/UIBlock.h>

#include "WrapperUIBlock.h"


void WrapperUIBlock::define(ClassDefinition& cls)
{
	WrapperUIElement::define(cls);
	cls.name = "UIBlock";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "size",  GetSize },
		{ "scissorEnabled",  GetScissorEnabled, SetScissorEnabled },		
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getSize", GetSize },
		{"setSize", SetSize },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}


void* WrapperUIBlock::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new UIBlock;
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


