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
	UIArea* self = new UIArea();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
}



void WrapperUIArea::GetElements(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> elements;
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_elements").ToLocalChecked()).ToChecked())
	{
		elements = holder->Get(context, v8::String::NewFromUtf8(isolate, "_elements").ToLocalChecked()).ToLocalChecked();
	}
	else
	{
		elements = v8::Array::New(isolate);
		holder->Set(context, v8::String::NewFromUtf8(isolate, "_elements").ToLocalChecked(), elements);
	}
	info.GetReturnValue().Set(elements);
}


void WrapperUIArea::Add(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIArea* self = (UIArea*)holder->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_element = info[0].As<v8::Object>();
	UIElement* element = (UIElement*)holder_element->GetAlignedPointerFromInternalField(0);

	self->add(element);

	v8::Local<v8::Array> elements = holder->Get(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	elements->Set(context, elements->Length(), holder_element);

	info.GetReturnValue().Set(holder);
}

void WrapperUIArea::Remove(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIArea* self = (UIArea*)holder->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_element = info[0].As<v8::Object>();
	UIElement* element = (UIElement*)holder_element->GetAlignedPointerFromInternalField(0);

	self->remove(element);

	v8::Local<v8::Array> elements = holder->Get(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToLocalChecked().As<v8::Array>();

	for (unsigned i = 0; i < elements->Length(); i++)
	{
		v8::Local<v8::Object> element_i = elements->Get(context, i).ToLocalChecked().As<v8::Object>();
		if (element_i == holder_element)
		{
			for (unsigned j = i; j < elements->Length() - 1; j++)
			{
				elements->Set(context, j, elements->Get(context, j + 1).ToLocalChecked());
			}
			elements->Delete(context, elements->Length() - 1);
			elements->Set(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked(), v8::Number::New(isolate, (double)(elements->Length() - 1)));
			break;
		}
	}

	info.GetReturnValue().Set(holder);
}

void WrapperUIArea::Clear(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIArea* self = (UIArea*)holder->GetAlignedPointerFromInternalField(0);
	self->clear();

	v8::Local<v8::Array> elements = holder->Get(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	for (unsigned i = 0; i < elements->Length(); i++)
	{
		elements->Delete(context, i);
	}
	elements->Set(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked(), v8::Number::New(isolate, 0.0));

	info.GetReturnValue().Set(holder);

}

void WrapperUIArea::GetViewers(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> viewers;
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_viewers").ToLocalChecked()).ToChecked())
	{
		viewers = holder->Get(context, v8::String::NewFromUtf8(isolate, "_viewers").ToLocalChecked()).ToLocalChecked();
	}
	else
	{
		viewers = v8::Array::New(isolate);
		holder->Set(context, v8::String::NewFromUtf8(isolate, "_viewers").ToLocalChecked(), viewers);
	}
	info.GetReturnValue().Set(viewers);
}

void WrapperUIArea::AddViewer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIArea* self = (UIArea*)holder->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_viewer = info[0].As<v8::Object>();
	UI3DViewer* viewer = (UI3DViewer*)holder_viewer->GetAlignedPointerFromInternalField(0);

	self->viewers.push_back(viewer);

	v8::Local<v8::Array> viewers = holder->Get(context, v8::String::NewFromUtf8(isolate, "viewers").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	viewers->Set(context, viewers->Length(), holder_viewer);

	info.GetReturnValue().Set(holder);
}

void WrapperUIArea::RemoveViewer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIArea* self = (UIArea*)holder->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_viewer = info[0].As<v8::Object>();
	UI3DViewer* viewer = (UI3DViewer*)holder_viewer->GetAlignedPointerFromInternalField(0);

	auto iter = std::find(self->viewers.begin(), self->viewers.end(), viewer);
	if (iter != self->viewers.end()) self->viewers.erase(iter);

	v8::Local<v8::Array> viewers = holder->Get(context, v8::String::NewFromUtf8(isolate, "viewers").ToLocalChecked()).ToLocalChecked().As<v8::Array>();

	for (unsigned i = 0; i < viewers->Length(); i++)
	{
		v8::Local<v8::Object> viewer_i = viewers->Get(context, i).ToLocalChecked().As<v8::Object>();
		if (viewer_i == holder_viewer)
		{
			for (unsigned j = i; j < viewers->Length() - 1; j++)
			{
				viewers->Set(context, j, viewers->Get(context, j + 1).ToLocalChecked());
			}
			viewers->Delete(context, viewers->Length() - 1);
			viewers->Set(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked(), v8::Number::New(isolate, (double)(viewers->Length() - 1)));
			break;
		}
	}

	info.GetReturnValue().Set(holder);
}

void WrapperUIArea::ClearViewers(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIArea* self = (UIArea*)holder->GetAlignedPointerFromInternalField(0);
	self->viewers.clear();

	v8::Local<v8::Array> viewers = holder->Get(context, v8::String::NewFromUtf8(isolate, "viewers").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	for (unsigned i = 0; i < viewers->Length(); i++)
	{
		viewers->Delete(context, i);
	}
	viewers->Set(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked(), v8::Number::New(isolate, 0.0));

	info.GetReturnValue().Set(holder);
}

void WrapperUIArea::GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UIArea* self = get_self<UIArea>(info);
	v8::Local<v8::Object> origin = v8::Object::New(isolate);
	vec2_to_jvec2(isolate, self->origin, origin);
	info.GetReturnValue().Set(origin);
}


void WrapperUIArea::GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIArea* self = get_self<UIArea>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec2_to_jvec2(isolate, self->origin, out);
	info.GetReturnValue().Set(out);
}


void WrapperUIArea::SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIArea* self = get_self<UIArea>(info);
	glm::vec2 origin;
	if (info[0]->IsNumber())
	{
		origin.x = (float)info[0].As<v8::Number>()->Value();
		origin.y = (float)info[1].As<v8::Number>()->Value();	
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec2_to_vec2(isolate, in, origin);
	}
	if (self->origin != origin)
	{
		self->origin = origin;
		self->appearance_changed = true;
	}
}

void WrapperUIArea::GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UIArea* self = get_self<UIArea>(info);
	v8::Local<v8::Object> size = v8::Object::New(isolate);
	vec2_to_jvec2(isolate, self->size, size);
	info.GetReturnValue().Set(size);
}

void WrapperUIArea::GetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIArea* self = get_self<UIArea>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec2_to_jvec2(isolate, self->size, out);
	info.GetReturnValue().Set(out);
}

void WrapperUIArea::SetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIArea* self = get_self<UIArea>(info);
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

void WrapperUIArea::GetScale(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	UIArea* self = get_self<UIArea>(info);
	v8::Local<v8::Number> scale = v8::Number::New(isolate, (double)self->scale);
	info.GetReturnValue().Set(scale);
}

void WrapperUIArea::SetScale(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	UIArea* self = get_self<UIArea>(info);
	float scale = (float)value.As<v8::Number>()->Value();
	if (self->scale != scale)
	{
		self->scale = scale;
		self->appearance_changed = true;
	}
}
