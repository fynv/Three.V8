#include "WrapperUtils.hpp"
#include "WrapperUIBlock.h"
#include <gui/UIScrollViewer.h>

#include "WrapperUIScrollViewer.h"


void WrapperUIScrollViewer::define(ClassDefinition& cls)
{
	WrapperUIBlock::define(cls);
	cls.name = "UIScrollViewer";
	cls.ctor = ctor;	

	std::vector<AccessorDefinition> props = {
		{ "scrollableVertical", GetScrollableVertical, SetScrollableVertical },
		{ "scrollableHorizontal", GetScrollableHorizontal, SetScrollableHorizontal },
		{ "scrollPosition", GetScrollPosition },
		{ "contentSize",GetContentSize },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getScrollPosition", GetScrollPosition },
		{"setScrollPosition", SetScrollPosition },
		{"getContentSize", GetContentSize },
		{"setContentSize", SetContentSize },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}

void* WrapperUIScrollViewer::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new UIScrollViewer;
}

void WrapperUIScrollViewer::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();

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

void WrapperUIScrollViewer::GetScrollableVertical(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	v8::Local<v8::Boolean> ret = v8::Boolean::New(lctx.isolate, self->scrollable_vertical);
	info.GetReturnValue().Set(ret);
}

void WrapperUIScrollViewer::SetScrollableVertical(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	self->scrollable_vertical = value.As<v8::Boolean>()->Value();
}

void WrapperUIScrollViewer::GetScrollableHorizontal(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	v8::Local<v8::Boolean> ret = v8::Boolean::New(lctx.isolate, self->scrollable_horizontal);
	info.GetReturnValue().Set(ret);
}

void WrapperUIScrollViewer::SetScrollableHorizontal(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	self->scrollable_horizontal = value.As<v8::Boolean>()->Value();
}


void WrapperUIScrollViewer::GetScrollPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	v8::Local<v8::Object> scrollPosition = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->scroll_position, scrollPosition);
	info.GetReturnValue().Set(scrollPosition);
}


void WrapperUIScrollViewer::GetScrollPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	lctx.vec2_to_jvec2(self->scroll_position, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUIScrollViewer::SetScrollPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	glm::vec2 scroll_position;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], scroll_position.x);
		lctx.jnum_to_num(info[1], scroll_position.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], scroll_position);
	}
	if (self->scroll_position != scroll_position)
	{
		self->scroll_position = scroll_position;
		self->appearance_changed = true;
	}
}

void WrapperUIScrollViewer::GetContentSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	v8::Local<v8::Object> contentSize = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->content_size, contentSize);
	info.GetReturnValue().Set(contentSize);
}


void WrapperUIScrollViewer::GetContentSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	lctx.vec2_to_jvec2(self->content_size, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUIScrollViewer::SetContentSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIScrollViewer* self = lctx.self<UIScrollViewer>();
	glm::vec2 content_size;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], content_size.x);
		lctx.jnum_to_num(info[1], content_size.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], content_size);
	}
	if (self->content_size != content_size)
	{
		self->content_size = content_size;
		self->appearance_changed = true;
	}
}

