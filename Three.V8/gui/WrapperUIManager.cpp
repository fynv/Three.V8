#include "WrapperUIManager.h"

#include "WrapperUtils.hpp"
#include <gui/UIManager.h>


void WrapperUIManager::define(ObjectDefinition& object)
{
	object.name = "UIManager";
	object.ctor = ctor;
	object.dtor = dtor;
	object.properties = {
		{ "areas", GetAreas },	
	};

	object.methods = {
		{ "add", Add },
		{ "remove", Remove },
		{ "clear", Clear },
	};
}

void* WrapperUIManager::ctor()
{
	return new UIManager;
}

void WrapperUIManager::dtor(void* ptr, GameContext* ctx)
{
	delete (UIManager*)ptr;
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

