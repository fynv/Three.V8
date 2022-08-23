#pragma once

#include "WrapperUtils.hpp"
#include "UIElement.hpp"
#include <gui/UILineEdit.h>

class WrapperUILineEdit
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetSize(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetText(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetText(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperUILineEdit::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperUIElement::create_template(isolate, constructor);	
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "size").ToLocalChecked(), GetSize, 0);
	templ->InstanceTemplate()->Set(isolate, "getSize", v8::FunctionTemplate::New(isolate, GetSize));
	templ->InstanceTemplate()->Set(isolate, "setSize", v8::FunctionTemplate::New(isolate, SetSize));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "text").ToLocalChecked(), GetText, SetText);
	templ->InstanceTemplate()->Set(isolate, "setStyle", v8::FunctionTemplate::New(isolate, SetStyle));


	return templ;
}

void WrapperUILineEdit::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	UILineEdit* self = new UILineEdit();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
}


void WrapperUILineEdit::dtor(void* ptr, GameContext* ctx)
{
	UILineEdit* self = (UILineEdit*)ptr;	
	delete self;
}



void WrapperUILineEdit::GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UILineEdit* self = get_self<UILineEdit>(info);
	v8::Local<v8::Object> size = v8::Object::New(isolate);
	vec2_to_jvec2(isolate, self->size, size);
	info.GetReturnValue().Set(size);
}


void WrapperUILineEdit::GetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UILineEdit* self = get_self<UILineEdit>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec2_to_jvec2(isolate, self->size, out);
	info.GetReturnValue().Set(out);
}


void WrapperUILineEdit::SetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UILineEdit* self = get_self<UILineEdit>(info);
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


void WrapperUILineEdit::GetText(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	UILineEdit* self = get_self<UILineEdit>(info);
	v8::Local<v8::String> ret = v8::String::NewFromUtf8(info.GetIsolate(), self->text.c_str()).ToLocalChecked();
	info.GetReturnValue().Set(ret);
}

void WrapperUILineEdit::SetText(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	UILineEdit* self = get_self<UILineEdit>(info);
	v8::String::Utf8Value text(info.GetIsolate(), value);
	self->text = *text;
	self->appearance_changed = true;
}

void WrapperUILineEdit::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	UILineEdit* self = get_self<UILineEdit>(info);

	v8::Local<v8::Object> style = info[0].As<v8::Object>();

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

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "colorBg").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Value> value = style->Get(context, v8::String::NewFromUtf8(isolate, "colorBg").ToLocalChecked()).ToLocalChecked();
		v8::String::Utf8Value str(info.GetIsolate(), value);
		string_to_color(isolate, *str, self->colorBg);
	}

	if (style->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "colorFg").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Value> value = style->Get(context, v8::String::NewFromUtf8(isolate, "colorFg").ToLocalChecked()).ToLocalChecked();
		v8::String::Utf8Value str(info.GetIsolate(), value);
		string_to_color(isolate, *str, self->colorFg);
	}

	self->appearance_changed = true;
}

