#include "WrapperUtils.hpp"
#include "core/WrapperObject3D.h"
#include <scenes/Scene.h>
#include <backgrounds/Background.h>
#include <lights/IndirectLight.h>
#include <scenes/Fog.h>

#include "WrapperScene.h"

void WrapperScene::define(ClassDefinition& cls)
{
	WrapperObject3D::define(cls);
	cls.name = "Scene";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "background", GetBackground, SetBackground },
		{ "indirectLight", GetIndirectLight, SetIndirectLight },
		{ "fog", GetFog, SetFog },
		{ "widgets", GetWidgets },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"addWidget", AddWidget },
		{"removeWidget", RemoveWidget },
		{"clearWidget", ClearWidgets },
		{"getBoundingBox", GetBoundingBox },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}


void* WrapperScene::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new Scene;
}


void WrapperScene::GetBackground(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> background = lctx.get_property(info.Holder(), "_background");
	info.GetReturnValue().Set(background);
}

void WrapperScene::SetBackground(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();
	if (!value->IsNull())
	{
		Background* background = lctx.jobj_to_obj<Background>(value);
		self->background = background;
	}
	else
	{
		self->background = nullptr;
	}
	lctx.set_property(info.Holder(), "_background", value);

}


void WrapperScene::GetIndirectLight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> indirectLight = lctx.get_property(info.Holder(), "_indirectLight");
	info.GetReturnValue().Set(indirectLight);
}

void WrapperScene::SetIndirectLight(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();
	if (!value->IsNull())
	{
		IndirectLight* indirectLight = lctx.jobj_to_obj<IndirectLight>(value);
		self->indirectLight = indirectLight;
	}
	else
	{
		self->indirectLight = nullptr;
	}
	lctx.set_property(info.Holder(), "_indirectLight", value);
}


void WrapperScene::GetFog(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> fog = lctx.get_property(info.Holder(), "_fog");
	info.GetReturnValue().Set(fog);
}

void WrapperScene::SetFog(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();
	if (!value->IsNull())
	{
		Fog* fog = lctx.jobj_to_obj<Fog>(value);
		self->fog = fog;
	}
	else
	{
		self->fog = nullptr;
	}
	lctx.set_property(info.Holder(), "_fog", value);
}



void WrapperScene::GetWidgets(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> widgets = lctx.get_property(info.Holder(), "_widgets");
	if (widgets->IsNull())
	{
		widgets = v8::Array::New(lctx.isolate);
		lctx.set_property(info.Holder(), "_widgets", widgets);
	}
	info.GetReturnValue().Set(widgets);
}


void WrapperScene::AddWidget(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();
	Object3D* object = lctx.jobj_to_obj<Object3D>(info[0]);
	self->add_widget(object);

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Object> holder_object = info[0].As<v8::Object>();

	v8::Local<v8::Array> widgets = lctx.get_property(holder, "widgets").As<v8::Array>();
	widgets->Set(lctx.context, widgets->Length(), holder_object);
}

void WrapperScene::RemoveWidget(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();
	Object3D* object = lctx.jobj_to_obj<Object3D>(info[0]);
	self->remove_widget(object);

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Object> holder_object = info[0].As<v8::Object>();

	v8::Local<v8::Array> widgets = lctx.get_property(holder, "widgets").As<v8::Array>();

	for (unsigned i = 0; i < widgets->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = widgets->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		if (obj_i == holder_object)
		{
			for (unsigned j = i; j < widgets->Length() - 1; j++)
			{
				widgets->Set(lctx.context, j, widgets->Get(lctx.context, j + 1).ToLocalChecked());
			}
			widgets->Delete(lctx.context, widgets->Length() - 1);
			lctx.set_property(widgets, "length", lctx.num_to_jnum(widgets->Length() - 1));
			break;
		}
	}
}


void WrapperScene::ClearWidgets(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();
	self->clear_widgets();

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Array> widgets = lctx.get_property(holder, "widgets").As<v8::Array>();

	for (unsigned i = 0; i < widgets->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = widgets->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		widgets->Delete(lctx.context, i);
	}
	lctx.set_property(widgets, "length", lctx.num_to_jnum(0));
}


void WrapperScene::GetBoundingBox(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Scene* self = lctx.self<Scene>();

	glm::vec3 min_pos, max_pos;
	self->get_bounding_box(min_pos, max_pos);

	v8::Local<v8::Object> ret = v8::Object::New(lctx.isolate);
	v8::Local<v8::Object> j_min_pos = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(min_pos, j_min_pos);
	v8::Local<v8::Object> j_max_pos = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(max_pos, j_max_pos);
	lctx.set_property(ret, "minPos", j_min_pos);
	lctx.set_property(ret, "maxPos", j_max_pos);
	info.GetReturnValue().Set(ret);
}
