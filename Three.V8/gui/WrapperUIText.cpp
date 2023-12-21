#include "WrapperUtils.hpp"
#include "WrapperUIElement.h"
#include <gui/UIText.h>

#include "WrapperUIText.h"

void WrapperUIText::define(ClassDefinition& cls)
{
	WrapperUIElement::define(cls);
	cls.name = "UIText";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "text", GetText, SetText },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"setStyle", SetStyle },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}

void* WrapperUIText::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new UIText;
}

void WrapperUIText::GetText(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIText* self = lctx.self<UIText>();
	info.GetReturnValue().Set(lctx.str_to_jstr(self->text.c_str()));
}

void WrapperUIText::SetText(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UIText* self = lctx.self<UIText>();
	std::string text = lctx.jstr_to_str(value);
	self->text = text;
	self->appearance_changed = true;
}

void WrapperUIText::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIText* self = lctx.self<UIText>();

	v8::Local<v8::Object> style = info[0].As<v8::Object>();

	if (lctx.has_property(style, "fontSize"))
	{
		lctx.jnum_to_num(lctx.get_property(style, "fontSize"), self->font_size);
	}

	if (lctx.has_property(style, "fontFace"))
	{
		self->font_face = lctx.jstr_to_str(lctx.get_property(style, "fontFace"));
	}

	if (lctx.has_property(style, "alignmentHorizontal"))
	{
		lctx.jnum_to_num(lctx.get_property(style, "alignmentHorizontal"), self->alignment_horizontal);
	}

	if (lctx.has_property(style, "alignmentVertical"))
	{
		lctx.jnum_to_num(lctx.get_property(style, "alignmentVertical"), self->alignment_vertical);
	}

	if (lctx.has_property(style, "colorFg"))
	{
		std::string str = lctx.jstr_to_str(lctx.get_property(style, "colorFg"));
		string_to_color(str.c_str(), self->colorFg);
	}

	self->appearance_changed = true;
}


