#pragma once

#include "WrapperUtils.hpp"
#include "UIBlock.hpp"
#include <gui/UIPanel.h>


class WrapperUIPanel
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperUIPanel::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperUIBlock::create_template(isolate, constructor);
	templ->InstanceTemplate()->Set(isolate, "setStyle", v8::FunctionTemplate::New(isolate, SetStyle));	
	return templ;
}

void WrapperUIPanel::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	UIPanel* self = new UIPanel();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), WrapperUIElement::dtor);
}

void WrapperUIPanel::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	UIPanel* self = get_self<UIPanel>(info);

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

