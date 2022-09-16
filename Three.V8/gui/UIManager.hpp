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
	LocalContext lctx(info);
	v8::Local<v8::Value> areas;
	if (lctx.has_property(lctx.holder, "_areas"))
	{
		areas = lctx.get_property(lctx.holder, "_areas");
	}
	else
	{
		areas = v8::Array::New(lctx.isolate);
		lctx.set_property(lctx.holder, "_areas", areas);
	}
	info.GetReturnValue().Set(areas);
}


void WrapperUIManager::Add(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIManager* self = lctx.self<UIManager>();
	UIArea* area = lctx.jobj_to_obj<UIArea>(info[0]);
	self->add(area);

	v8::Local<v8::Array> areas = lctx.get_property(lctx.holder, "areas").As<v8::Array>();
	areas->Set(lctx.context, areas->Length(), info[0]);

	info.GetReturnValue().Set(lctx.holder);
}

void WrapperUIManager::Remove(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIManager* self = lctx.self<UIManager>();
	UIArea* area = lctx.jobj_to_obj<UIArea>(info[0]);
	self->remove(area);

	v8::Local<v8::Array> areas = lctx.get_property(lctx.holder, "areas").As<v8::Array>();

	for (unsigned i = 0; i < areas->Length(); i++)
	{
		v8::Local<v8::Value> area_i = areas->Get(lctx.context, i).ToLocalChecked();
		if (area_i == info[0])
		{			
			for (unsigned j = i; j < areas->Length() - 1; j++)
			{
				areas->Set(lctx.context, j, areas->Get(lctx.context, j + 1).ToLocalChecked());
			}
			areas->Delete(lctx.context, areas->Length() - 1);
			lctx.set_property(areas, "length", lctx.num_to_jnum(areas->Length() - 1));
			break;
		}
	}

	info.GetReturnValue().Set(lctx.holder);
}

void WrapperUIManager::Clear(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIManager* self = lctx.self<UIManager>();
	self->clear();

	v8::Local<v8::Array> areas = lctx.get_property(lctx.holder, "areas").As<v8::Array>();
	for (unsigned i = 0; i < areas->Length(); i++)
	{		
		areas->Delete(lctx.context, i);
	}
	lctx.set_property(areas, "length", lctx.num_to_jnum(0));
	info.GetReturnValue().Set(lctx.holder);

}