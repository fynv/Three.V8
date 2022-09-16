#pragma once

#include "WrapperUtils.hpp"
#include <gui/UIArea.h>


class WrapperUIArea
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void GetElements(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Add(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Remove(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Clear(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetViewers(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void AddViewer(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RemoveViewer(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void ClearViewers(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetSize(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetScale(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);	
	static void SetScale(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};

v8::Local<v8::FunctionTemplate> WrapperUIArea::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);

	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked(), GetElements, 0);
	templ->InstanceTemplate()->Set(isolate, "add", v8::FunctionTemplate::New(isolate, Add));
	templ->InstanceTemplate()->Set(isolate, "remove", v8::FunctionTemplate::New(isolate, Remove));	
	templ->InstanceTemplate()->Set(isolate, "clear", v8::FunctionTemplate::New(isolate, Clear));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "viewers").ToLocalChecked(), GetViewers, 0);
	templ->InstanceTemplate()->Set(isolate, "addViewer", v8::FunctionTemplate::New(isolate, AddViewer));
	templ->InstanceTemplate()->Set(isolate, "removeViewer", v8::FunctionTemplate::New(isolate, RemoveViewer));
	templ->InstanceTemplate()->Set(isolate, "clearViewer", v8::FunctionTemplate::New(isolate, ClearViewers));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "origin").ToLocalChecked(), GetOrigin, 0);
	templ->InstanceTemplate()->Set(isolate, "getOrigin", v8::FunctionTemplate::New(isolate, GetOrigin));
	templ->InstanceTemplate()->Set(isolate, "setOrigin", v8::FunctionTemplate::New(isolate, SetOrigin));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "size").ToLocalChecked(), GetSize, 0);
	templ->InstanceTemplate()->Set(isolate, "getSize", v8::FunctionTemplate::New(isolate, GetSize));
	templ->InstanceTemplate()->Set(isolate, "setSize", v8::FunctionTemplate::New(isolate, SetSize));	

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "scale").ToLocalChecked(), GetScale, SetScale);

	return templ;

}

void WrapperUIArea::dtor(void* ptr, GameContext* ctx)
{
	delete (UIArea*)ptr;
}

void WrapperUIArea::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIArea* self = new UIArea();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
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
