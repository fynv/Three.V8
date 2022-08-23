#pragma once

#include "WrapperUtils.hpp"
#include <gui/UIManager.h>

class WrapperUIManager
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void GetAreas(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Add(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Remove(const v8::FunctionCallbackInfo<v8::Value>& info);	
	static void Clear(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::ObjectTemplate> WrapperUIManager::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->SetInternalFieldCount(1);
	templ->SetAccessor(v8::String::NewFromUtf8(isolate, "areas").ToLocalChecked(), GetAreas, 0);
	templ->Set(isolate, "add", v8::FunctionTemplate::New(isolate, Add));
	templ->Set(isolate, "remove", v8::FunctionTemplate::New(isolate, Remove));
	templ->Set(isolate, "clear", v8::FunctionTemplate::New(isolate, Clear));
	return templ;
}


void WrapperUIManager::GetAreas(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> areas;
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_areas").ToLocalChecked()).ToChecked())
	{
		areas = holder->Get(context, v8::String::NewFromUtf8(isolate, "_areas").ToLocalChecked()).ToLocalChecked();
	}
	else
	{
		areas = v8::Array::New(isolate);
		holder->Set(context, v8::String::NewFromUtf8(isolate, "_areas").ToLocalChecked(), areas);
	}
	info.GetReturnValue().Set(areas);
}


void WrapperUIManager::Add(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIManager* self = (UIManager*)holder->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_area = info[0].As<v8::Object>();
	UIArea* area = (UIArea*)holder_area->GetAlignedPointerFromInternalField(0);

	self->add(area);

	v8::Local<v8::Array> areas = holder->Get(context, v8::String::NewFromUtf8(isolate, "areas").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	areas->Set(context, areas->Length(), holder_area);

	info.GetReturnValue().Set(holder);
}

void WrapperUIManager::Remove(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIManager* self = (UIManager*)holder->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::Object> holder_area = info[0].As<v8::Object>();
	UIArea* area = (UIArea*)holder_area->GetAlignedPointerFromInternalField(0);

	self->remove(area);

	v8::Local<v8::Array> areas = holder->Get(context, v8::String::NewFromUtf8(isolate, "areas").ToLocalChecked()).ToLocalChecked().As<v8::Array>();

	for (unsigned i = 0; i < areas->Length(); i++)
	{
		v8::Local<v8::Object> area_i = areas->Get(context, i).ToLocalChecked().As<v8::Object>();
		if (area_i == holder_area)
		{			
			for (unsigned j = i; j < areas->Length() - 1; j++)
			{
				areas->Set(context, j, areas->Get(context, j + 1).ToLocalChecked());
			}
			areas->Delete(context, areas->Length() - 1);
			areas->Set(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked(), v8::Number::New(isolate, (double)(areas->Length() - 1)));
			break;
		}
	}

	info.GetReturnValue().Set(holder);
}

void WrapperUIManager::Clear(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	UIManager* self = (UIManager*)holder->GetAlignedPointerFromInternalField(0);
	self->clear();

	v8::Local<v8::Array> areas = holder->Get(context, v8::String::NewFromUtf8(isolate, "areas").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	for (unsigned i = 0; i < areas->Length(); i++)
	{		
		areas->Delete(context, i);
	}
	areas->Set(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked(), v8::Number::New(isolate, 0.0));

	info.GetReturnValue().Set(holder);

}