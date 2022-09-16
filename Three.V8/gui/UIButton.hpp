#pragma once

#include "WrapperUtils.hpp"
#include "UIBlock.hpp"
#include <gui/UIButton.h>


class WrapperUIButton
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info);
	
	static void GetOnClick(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnClick(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOnLongPress(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnLongPress(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};


v8::Local<v8::FunctionTemplate> WrapperUIButton::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperUIBlock::create_template(isolate, constructor);
	templ->InstanceTemplate()->Set(isolate, "setStyle", v8::FunctionTemplate::New(isolate, SetStyle));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "onClick").ToLocalChecked(), GetOnClick, SetOnClick);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "onLongPress").ToLocalChecked(), GetOnLongPress, SetOnLongPress);
	return templ;
}

void WrapperUIButton::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIButton* self = new UIButton();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
}

typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;

struct UIButtonClickData
{
	GameContext* ctx;
	CallbackT callback;
};

void WrapperUIButton::dtor(void* ptr, GameContext* ctx)
{
	UIButton* self = (UIButton*)ptr;
	if (self->click_callback_data != nullptr)
	{
		UIButtonClickData* data = (UIButtonClickData*)self->click_callback_data;
		delete data;
	}
	if (self->long_press_callback_data != nullptr)
	{
		UIButtonClickData* data = (UIButtonClickData*)self->long_press_callback_data;
		delete data;
	}
	delete self;
}

void WrapperUIButton::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIButton* self = lctx.self<UIButton>();

	v8::Local<v8::Object> style = info[0].As<v8::Object>();

	if (lctx.has_property(style, "cornerRadius"))
	{
		lctx.jnum_to_num(lctx.get_property(style, "cornerRadius"), self->cornerRadius);
	}

	if (lctx.has_property(style, "strokeWidth"))
	{
		lctx.jnum_to_num(lctx.get_property(style, "strokeWidth"), self->strokeWidth);
	}

	if (lctx.has_property(style, "colorBg"))
	{
		std::string str = lctx.jstr_to_str(lctx.get_property(style, "colorBg"));
		string_to_color(str.c_str(), self->colorBg);
	}

	if (lctx.has_property(style, "colorStroke"))
	{
		std::string str = lctx.jstr_to_str(lctx.get_property(style, "colorStroke"));
		string_to_color(str.c_str(), self->colorStroke);
	}
	self->appearance_changed = true;
}


static void UIButtonClickCallback(void* ptr)
{
	UIButtonClickData* data = (UIButtonClickData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);	
	v8::Local<v8::Object> global = context->Global();
	callback->Call(context, global, 0, nullptr);
}

void WrapperUIButton::GetOnClick(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onClick = lctx.get_property(lctx.holder, "_onClick");
	info.GetReturnValue().Set(onClick);
}

void WrapperUIButton::SetOnClick(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onClick", value);

	UIButton* self = lctx.self<UIButton>();
	self->click_callback = UIButtonClickCallback;

	if (self->click_callback_data != nullptr)
	{
		UIButtonClickData* data = (UIButtonClickData*)self->click_callback_data;
		delete data;	
	}

	UIButtonClickData* data = new UIButtonClickData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->click_callback_data = data;

}


void WrapperUIButton::GetOnLongPress(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);	
	v8::Local<v8::Value> onLongPress = lctx.get_property(lctx.holder, "_onLongPress");
	info.GetReturnValue().Set(onLongPress);
}

void WrapperUIButton::SetOnLongPress(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onLongPress", value);

	UIButton* self = lctx.self<UIButton>();
	self->long_press_callback = UIButtonClickCallback;

	if (self->long_press_callback_data != nullptr)
	{
		UIButtonClickData* data = (UIButtonClickData*)self->long_press_callback_data;
		delete data;
	}

	UIButtonClickData* data = new UIButtonClickData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->long_press_callback_data = data;

}