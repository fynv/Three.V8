#include "WrapperUtils.hpp"
#include "WrapperUIElement.h"
#include <gui/UILineEdit.h>

#include "WrapperUILineEdit.h"

void WrapperUILineEdit::define(ClassDefinition& cls)
{
	WrapperUIElement::define(cls);
	cls.name = "UILineEdit";
	cls.ctor = ctor;
	cls.dtor = dtor;

	std::vector<AccessorDefinition> props = {
		{ "size", GetSize },
		{ "text",GetText, SetText },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getSize", GetSize },
		{"setSize", SetSize },
		{"setStyle", SetStyle },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}


void* WrapperUILineEdit::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new UILineEdit;
}

void WrapperUILineEdit::dtor(void* ptr, GameContext* ctx)
{
	UILineEdit* self = (UILineEdit*)ptr;
	WrapperUIElement::dtor(ptr, ctx);
}

void WrapperUILineEdit::GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UILineEdit* self = lctx.self<UILineEdit>();
	v8::Local<v8::Object> size = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->size, size);
	info.GetReturnValue().Set(size);
}


void WrapperUILineEdit::GetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UILineEdit* self = lctx.self<UILineEdit>();
	lctx.vec2_to_jvec2(self->size, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUILineEdit::SetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UILineEdit* self = lctx.self<UILineEdit>();
	glm::vec2 size;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], size.x);
		lctx.jnum_to_num(info[1], size.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], size);
	}
	if (self->size != size)
	{
		self->size = size;
		self->appearance_changed = true;
	}
}


void WrapperUILineEdit::GetText(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UILineEdit* self = lctx.self<UILineEdit>();
	info.GetReturnValue().Set(lctx.str_to_jstr(self->text.c_str()));
}

void WrapperUILineEdit::SetText(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UILineEdit* self = lctx.self<UILineEdit>();
	std::string text = lctx.jstr_to_str(value);
	self->text = text;
	self->appearance_changed = true;
}

void WrapperUILineEdit::SetStyle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UILineEdit* self = lctx.self<UILineEdit>();

	v8::Local<v8::Object> style = info[0].As<v8::Object>();

	if (lctx.has_property(style, "fontSize"))
	{
		lctx.jnum_to_num(lctx.get_property(style, "fontSize"), self->font_size);
	}

	if (lctx.has_property(style, "fontFace"))
	{
		self->font_face = lctx.jstr_to_str(lctx.get_property(style, "fontFace"));
	}

	if (lctx.has_property(style, "colorBg"))
	{
		std::string str = lctx.jstr_to_str(lctx.get_property(style, "colorBg"));
		string_to_color(str.c_str(), self->colorBg);
	}

	if (lctx.has_property(style, "colorFg"))
	{
		std::string str = lctx.jstr_to_str(lctx.get_property(style, "colorFg"));
		string_to_color(str.c_str(), self->colorFg);
	}

	self->appearance_changed = true;
}





