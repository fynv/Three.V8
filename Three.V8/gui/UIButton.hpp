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
	UIButton* self = new UIButton();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
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
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	UIButton* self = get_self<UIButton>(info);

	v8::Local<v8::Object> style = info[0].As<v8::Object>();

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "cornerRadius").ToLocalChecked()).ToChecked())
	{
		self->cornerRadius = (float)style->Get(context, v8::String::NewFromUtf8(isolate, "cornerRadius").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "strokeWidth").ToLocalChecked()).ToChecked())
	{
		self->strokeWidth = (float)style->Get(context, v8::String::NewFromUtf8(isolate, "strokeWidth").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "colorBg").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Value> value = style->Get(context, v8::String::NewFromUtf8(isolate, "colorBg").ToLocalChecked()).ToLocalChecked();
		v8::String::Utf8Value str(info.GetIsolate(), value);
		string_to_color(isolate, *str, self->colorBg);
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "colorStroke").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Value> value = style->Get(context, v8::String::NewFromUtf8(isolate, "colorStroke").ToLocalChecked()).ToLocalChecked();
		v8::String::Utf8Value str(info.GetIsolate(), value);
		string_to_color(isolate, *str, self->colorStroke);
	}
	self->appearance_changed = true;
}


static void UIButtonClickCallback(void* ptr)
{
	UIButtonClickData* data = (UIButtonClickData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(ctx->m_context.Get(isolate));
	v8::Local<v8::Function> callback = data->callback.Get(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	callback->Call(context, global, 0, nullptr);
}

void WrapperUIButton::GetOnClick(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> onClick = v8::Null(isolate);
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_onClick").ToLocalChecked()).ToChecked())
	{
		onClick = holder->Get(context, v8::String::NewFromUtf8(isolate, "_onClick").ToLocalChecked()).ToLocalChecked();
	}
	info.GetReturnValue().Set(onClick);
}

void WrapperUIButton::SetOnClick(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();	
	holder->Set(context, v8::String::NewFromUtf8(isolate, "_onClick").ToLocalChecked(), value);


	UIButton* self = (UIButton*)holder->GetAlignedPointerFromInternalField(0);
	self->click_callback = UIButtonClickCallback;

	if (self->click_callback_data != nullptr)
	{
		UIButtonClickData* data = (UIButtonClickData*)self->click_callback_data;
		delete data;	
	}

	UIButtonClickData* data = new UIButtonClickData;
	data->ctx = get_context(info);

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(isolate, callback);

	self->click_callback_data = data;

}


void WrapperUIButton::GetOnLongPress(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> onLongPress = v8::Null(isolate);
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_onLongPress").ToLocalChecked()).ToChecked())
	{
		onLongPress = holder->Get(context, v8::String::NewFromUtf8(isolate, "_onLongPress").ToLocalChecked()).ToLocalChecked();
	}
	info.GetReturnValue().Set(onLongPress);
}

void WrapperUIButton::SetOnLongPress(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	holder->Set(context, v8::String::NewFromUtf8(isolate, "_onLongPress").ToLocalChecked(), value);


	UIButton* self = (UIButton*)holder->GetAlignedPointerFromInternalField(0);
	self->long_press_callback = UIButtonClickCallback;

	if (self->long_press_callback_data != nullptr)
	{
		UIButtonClickData* data = (UIButtonClickData*)self->long_press_callback_data;
		delete data;
	}

	UIButtonClickData* data = new UIButtonClickData;
	data->ctx = get_context(info);

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(isolate, callback);

	self->long_press_callback_data = data;

}