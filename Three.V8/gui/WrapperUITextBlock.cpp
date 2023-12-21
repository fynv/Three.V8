#include "WrapperUtils.hpp"
#include "WrapperUIElement.h"
#include <gui/UITextBlock.h>

#include "WrapperUITextBlock.h"

void WrapperUITextBlock::define(ClassDefinition& cls)
{
	WrapperUIElement::define(cls);
	cls.name = "UITextBlock";
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

void* WrapperUITextBlock::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new UITextBlock;
}

void WrapperUITextBlock::GetText(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UITextBlock* self = lctx.self<UITextBlock>();
	info.GetReturnValue().Set(lctx.str_to_jstr(self->text.c_str()));
}

void WrapperUITextBlock::SetText(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UITextBlock* self = lctx.self<UITextBlock>();
	std::string text = lctx.jstr_to_str(value);
	self->text = text;
	self->appearance_changed = true;
}

void WrapperUITextBlock::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UITextBlock* self = lctx.self<UITextBlock>();

	v8::Local<v8::Object> style = info[0].As<v8::Object>();

	if (lctx.has_property(style, "lineWidth"))
	{
		lctx.jnum_to_num(lctx.get_property(style, "lineWidth"), self->line_width);
	}

	if (lctx.has_property(style, "lineHeight"))
	{
		lctx.jnum_to_num(lctx.get_property(style, "lineHeight"), self->line_height);
	}

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


