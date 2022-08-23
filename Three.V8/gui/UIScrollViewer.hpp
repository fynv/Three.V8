#pragma once

#include "WrapperUtils.hpp"
#include "UIBlock.hpp"
#include <gui/UIScrollViewer.h>


class WrapperUIScrollViewer
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetScrollableVertical(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetScrollableVertical(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetScrollableHorizontal(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetScrollableHorizontal(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetScrollPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetScrollPosition(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetScrollPosition(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetContentSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetContentSize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetContentSize(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperUIScrollViewer::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperUIBlock::create_template(isolate, constructor);
	templ->InstanceTemplate()->Set(isolate, "setStyle", v8::FunctionTemplate::New(isolate, SetStyle));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "scrollableVertical").ToLocalChecked(), GetScrollableVertical, SetScrollableVertical);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "scrollableHorizontal").ToLocalChecked(), GetScrollableHorizontal, SetScrollableHorizontal);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "scrollPosition").ToLocalChecked(), GetScrollPosition, 0);
	templ->InstanceTemplate()->Set(isolate, "getScrollPosition", v8::FunctionTemplate::New(isolate, GetScrollPosition));
	templ->InstanceTemplate()->Set(isolate, "setScrollPosition", v8::FunctionTemplate::New(isolate, SetScrollPosition));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "contentSize").ToLocalChecked(), GetContentSize, 0);
	templ->InstanceTemplate()->Set(isolate, "getContentSize", v8::FunctionTemplate::New(isolate, GetContentSize));
	templ->InstanceTemplate()->Set(isolate, "setContentSize", v8::FunctionTemplate::New(isolate, SetContentSize));

	return templ;
}

void WrapperUIScrollViewer::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	UIScrollViewer* self = new UIScrollViewer();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), WrapperUIElement::dtor);
}

void WrapperUIScrollViewer::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	UIScrollViewer* self = get_self<UIScrollViewer>(info);

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

void WrapperUIScrollViewer::GetScrollableVertical(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	v8::Local<v8::Boolean> ret = v8::Boolean::New(isolate, self->scrollable_vertical);
	info.GetReturnValue().Set(ret);
}

void WrapperUIScrollViewer::SetScrollableVertical(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	self->scrollable_vertical = value.As<v8::Boolean>()->Value();
}

void WrapperUIScrollViewer::GetScrollableHorizontal(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	v8::Local<v8::Boolean> ret = v8::Boolean::New(isolate, self->scrollable_horizontal);
	info.GetReturnValue().Set(ret);
}

void WrapperUIScrollViewer::SetScrollableHorizontal(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	self->scrollable_horizontal = value.As<v8::Boolean>()->Value();
}


void WrapperUIScrollViewer::GetScrollPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	v8::Local<v8::Object> scrollPosition = v8::Object::New(isolate);
	vec2_to_jvec2(isolate, self->scroll_position, scrollPosition);
	info.GetReturnValue().Set(scrollPosition);
}


void WrapperUIScrollViewer::GetScrollPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec2_to_jvec2(isolate, self->scroll_position, out);
	info.GetReturnValue().Set(out);
}


void WrapperUIScrollViewer::SetScrollPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	glm::vec2 scroll_position;
	if (info[0]->IsNumber())
	{
		scroll_position.x = (float)info[0].As<v8::Number>()->Value();
		scroll_position.y = (float)info[1].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec2_to_vec2(isolate, in, scroll_position);
	}
	if (self->scroll_position != scroll_position)
	{
		self->scroll_position = scroll_position;
		self->appearance_changed = true;
	}
}

void WrapperUIScrollViewer::GetContentSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	v8::Local<v8::Object> contentSize = v8::Object::New(isolate);
	vec2_to_jvec2(isolate, self->content_size, contentSize);
	info.GetReturnValue().Set(contentSize);
}


void WrapperUIScrollViewer::GetContentSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec2_to_jvec2(isolate, self->content_size, out);
	info.GetReturnValue().Set(out);
}


void WrapperUIScrollViewer::SetContentSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIScrollViewer* self = get_self<UIScrollViewer>(info);
	glm::vec2 content_size;
	if (info[0]->IsNumber())
	{
		content_size.x = (float)info[0].As<v8::Number>()->Value();
		content_size.y = (float)info[1].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec2_to_vec2(isolate, in, content_size);
	}
	if (self->content_size != content_size)
	{
		self->content_size = content_size;
		self->appearance_changed = true;
	}	
}
