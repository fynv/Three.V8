#include "WrapperUtils.hpp"
#include <gui/UIArea.h>
#include "WrapperUIArea.h"

void WrapperUIArea::define(ClassDefinition& cls)
{
	cls.name = "UIArea";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "elements", GetElements},
		{ "viewers", GetViewers},
		{ "origin", GetOrigin },
		{ "size", GetSize },
		{ "scale", GetScale, SetScale },		
	};
	cls.methods = {
		{"add", Add },
		{"remove", Remove },
		{"clear", Clear },
		{"addViewer", AddViewer },
		{"removeViewer", RemoveViewer },
		{"clearViewer", ClearViewers },
		{"getOrigin", GetOrigin },
		{"setOrigin", SetOrigin },
		{"getSize", GetSize },
		{"setSize", SetSize },
	};
}

void* WrapperUIArea::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new UIArea;
}

void WrapperUIArea::dtor(void* ptr, GameContext* ctx)
{
	delete (UIArea*)ptr;
}

void WrapperUIArea::GetElements(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> elements;
	if (lctx.has_property(lctx.holder, "_elements"))
	{
		elements = lctx.get_property(lctx.holder, "_elements");
	}
	else
	{
		elements = v8::Array::New(lctx.isolate);
		lctx.set_property(lctx.holder, "_elements", elements);
	}
	info.GetReturnValue().Set(elements);
}


void WrapperUIArea::Add(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	UIElement* element = lctx.jobj_to_obj<UIElement>(info[0]);
	self->add(element);

	v8::Local<v8::Array> elements = lctx.get_property(lctx.holder, "elements").As<v8::Array>();
	elements->Set(lctx.context, elements->Length(), info[0]);

	info.GetReturnValue().Set(lctx.holder);
}

void WrapperUIArea::Remove(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	UIElement* element = lctx.jobj_to_obj<UIElement>(info[0]);
	self->remove(element);

	v8::Local<v8::Array> elements = lctx.get_property(lctx.holder, "elements").As<v8::Array>();

	for (unsigned i = 0; i < elements->Length(); i++)
	{
		v8::Local<v8::Value> element_i = elements->Get(lctx.context, i).ToLocalChecked();
		if (element_i == info[0])
		{
			for (unsigned j = i; j < elements->Length() - 1; j++)
			{
				elements->Set(lctx.context, j, elements->Get(lctx.context, j + 1).ToLocalChecked());
			}
			elements->Delete(lctx.context, elements->Length() - 1);
			lctx.set_property(elements, "length", lctx.num_to_jnum(elements->Length() - 1));
			break;
		}
	}

	info.GetReturnValue().Set(lctx.holder);
}

void WrapperUIArea::Clear(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	self->clear();

	v8::Local<v8::Array> elements = lctx.get_property(lctx.holder, "elements").As<v8::Array>();
	for (unsigned i = 0; i < elements->Length(); i++)
	{
		elements->Delete(lctx.context, i);
	}
	lctx.set_property(elements, "length", lctx.num_to_jnum(0));

	info.GetReturnValue().Set(lctx.holder);

}

void WrapperUIArea::GetViewers(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> viewers;
	if (lctx.has_property(lctx.holder, "_viewers"))
	{
		viewers = lctx.get_property(lctx.holder, "_viewers");
	}
	else
	{
		viewers = v8::Array::New(lctx.isolate);
		lctx.set_property(lctx.holder, "_viewers", viewers);
	}
	info.GetReturnValue().Set(viewers);
}

void WrapperUIArea::AddViewer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	UI3DViewer* viewer = lctx.jobj_to_obj<UI3DViewer>(info[0]);
	self->viewers.push_back(viewer);

	v8::Local<v8::Array> viewers = lctx.get_property(lctx.holder, "viewers").As<v8::Array>();
	viewers->Set(lctx.context, viewers->Length(), info[0]);

	info.GetReturnValue().Set(lctx.holder);
}

void WrapperUIArea::RemoveViewer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	UI3DViewer* viewer = lctx.jobj_to_obj<UI3DViewer>(info[0]);

	auto iter = std::find(self->viewers.begin(), self->viewers.end(), viewer);
	if (iter != self->viewers.end()) self->viewers.erase(iter);

	v8::Local<v8::Array> viewers = lctx.get_property(lctx.holder, "viewers").As<v8::Array>();

	for (unsigned i = 0; i < viewers->Length(); i++)
	{
		v8::Local<v8::Value> viewer_i = viewers->Get(lctx.context, i).ToLocalChecked();
		if (viewer_i == info[0])
		{
			for (unsigned j = i; j < viewers->Length() - 1; j++)
			{
				viewers->Set(lctx.context, j, viewers->Get(lctx.context, j + 1).ToLocalChecked());
			}
			viewers->Delete(lctx.context, viewers->Length() - 1);
			lctx.set_property(viewers, "length", lctx.num_to_jnum(viewers->Length() - 1));
			break;
		}
	}

	info.GetReturnValue().Set(lctx.holder);
}

void WrapperUIArea::ClearViewers(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	self->viewers.clear();

	v8::Local<v8::Array> viewers = lctx.get_property(lctx.holder, "viewers").As<v8::Array>();
	for (unsigned i = 0; i < viewers->Length(); i++)
	{
		viewers->Delete(lctx.context, i);
	}
	lctx.set_property(viewers, "length", lctx.num_to_jnum(0));

	info.GetReturnValue().Set(lctx.holder);
}

void WrapperUIArea::GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	v8::Local<v8::Object> origin = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->origin, origin);
	info.GetReturnValue().Set(origin);
}


void WrapperUIArea::GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	lctx.vec2_to_jvec2(self->origin, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUIArea::SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	glm::vec2 origin;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], origin.x);
		lctx.jnum_to_num(info[1], origin.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], origin);
	}

	if (self->origin != origin)
	{
		self->origin = origin;
		self->appearance_changed = true;
	}
}

void WrapperUIArea::GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	v8::Local<v8::Object> size = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->size, size);
	info.GetReturnValue().Set(size);
}

void WrapperUIArea::GetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	lctx.vec2_to_jvec2(self->size, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperUIArea::SetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
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

void WrapperUIArea::GetScale(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->scale));
}

void WrapperUIArea::SetScale(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UIArea* self = lctx.self<UIArea>();
	float scale;
	lctx.jnum_to_num(value, scale);
	if (self->scale != scale)
	{
		self->scale = scale;
		self->appearance_changed = true;
	}
}
