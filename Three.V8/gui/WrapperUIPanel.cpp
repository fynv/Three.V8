#include "WrapperUtils.hpp"
#include "WrapperUIBlock.h"
#include <gui/UIPanel.h>

#include "WrapperUIPanel.h"


void WrapperUIPanel::define(ClassDefinition& cls)
{
	WrapperUIBlock::define(cls);
	cls.name = "UIPanel";
	cls.ctor = ctor;

	std::vector<FunctionDefinition> methods = {
		{"setStyle", SetStyle },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}

void* WrapperUIPanel::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new UIPanel;
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

