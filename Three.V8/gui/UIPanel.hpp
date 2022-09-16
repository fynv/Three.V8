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
	LocalContext lctx(info);
	UIPanel* self = new UIPanel();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), WrapperUIElement::dtor);
}

void WrapperUIPanel::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIPanel* self = lctx.self<UIPanel>();

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

