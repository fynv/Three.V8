#pragma once

#include "WrapperUtils.hpp"
#include "UIElement.hpp"
#include <gui/UITextBlock.h>


class WrapperUITextBlock
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetText(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetText(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
	static void SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperUITextBlock::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperUIElement::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "text").ToLocalChecked(), GetText, SetText);
	templ->InstanceTemplate()->Set(isolate, "setStyle", v8::FunctionTemplate::New(isolate, SetStyle));
	return templ;
}

void WrapperUITextBlock::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	UITextBlock* self = new UITextBlock();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), WrapperUIElement::dtor);
}


void WrapperUITextBlock::GetText(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	UITextBlock* self = get_self<UITextBlock>(info);
	v8::Local<v8::String> ret = v8::String::NewFromUtf8(info.GetIsolate(), self->text.c_str()).ToLocalChecked();
	info.GetReturnValue().Set(ret);
}

void WrapperUITextBlock::SetText(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	UITextBlock* self = get_self<UITextBlock>(info);
	v8::String::Utf8Value text(info.GetIsolate(), value);
	self->text = *text;
	self->appearance_changed = true;
}

void WrapperUITextBlock::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	UITextBlock* self = get_self<UITextBlock>(info);

	v8::Local<v8::Object> style = info[0].As<v8::Object>();

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "lineWidth").ToLocalChecked()).ToChecked())
	{
		self->line_width = (float)style->Get(context, v8::String::NewFromUtf8(isolate, "lineWidth").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "lineHeight").ToLocalChecked()).ToChecked())
	{
		self->line_height = (float)style->Get(context, v8::String::NewFromUtf8(isolate, "lineHeight").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "fontSize").ToLocalChecked()).ToChecked())
	{
		self->font_size = (float)style->Get(context, v8::String::NewFromUtf8(isolate, "fontSize").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "fontFace").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Value> value = style->Get(context, v8::String::NewFromUtf8(isolate, "fontFace").ToLocalChecked()).ToLocalChecked();
		v8::String::Utf8Value fontFace(isolate, value);
		self->font_face = *fontFace;
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "alignmentHorizontal").ToLocalChecked()).ToChecked())
	{
		self->alignment_horizontal = (int)style->Get(context, v8::String::NewFromUtf8(isolate, "alignmentHorizontal").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "alignmentVertical").ToLocalChecked()).ToChecked())
	{
		self->alignment_vertical = (int)style->Get(context, v8::String::NewFromUtf8(isolate, "alignmentVertical").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "colorFg").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Value> value = style->Get(context, v8::String::NewFromUtf8(isolate, "colorFg").ToLocalChecked()).ToLocalChecked();
		v8::String::Utf8Value str(info.GetIsolate(), value);
		string_to_color(isolate, *str, self->colorFg);
	}

	self->appearance_changed = true;
}

