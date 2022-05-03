#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <models/SimpleModel.h>
#include <models/GeometryCreator.h>
#include <utils/Image.h>

class WrapperSimpleModel
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void CreateBox(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreateSphere(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreatePlane(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void SetColorTexture(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetMetalness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetMetalness(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);	

	static void GetRoughness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetRoughness(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void SetToonShading(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperSimpleModel::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);	
	templ->InstanceTemplate()->Set(isolate, "createBox", v8::FunctionTemplate::New(isolate, CreateBox));
	templ->InstanceTemplate()->Set(isolate, "createSphere", v8::FunctionTemplate::New(isolate, CreateSphere));
	templ->InstanceTemplate()->Set(isolate, "createPlane", v8::FunctionTemplate::New(isolate, CreatePlane));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "color").ToLocalChecked(), GetColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getColor", v8::FunctionTemplate::New(isolate, GetColor));
	templ->InstanceTemplate()->Set(isolate, "setColor", v8::FunctionTemplate::New(isolate, SetColor));

	templ->InstanceTemplate()->Set(isolate, "setColorTexture", v8::FunctionTemplate::New(isolate, SetColorTexture));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "metalness").ToLocalChecked(), GetMetalness, SetMetalness);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "roughness").ToLocalChecked(), GetRoughness, SetRoughness);

	templ->InstanceTemplate()->Set(isolate, "setToonShading", v8::FunctionTemplate::New(isolate, SetToonShading));

	return templ;
}

void WrapperSimpleModel::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	SimpleModel* self = new SimpleModel();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
}

void WrapperSimpleModel::CreateBox(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	SimpleModel* self = get_self<SimpleModel>(info);
	float width = (float)info[0].As<v8::Number>()->Value();
	float height = (float)info[1].As<v8::Number>()->Value();
	float depth = (float)info[2].As<v8::Number>()->Value();
	GeometryCreator::CreateBox(&self->geometry, width, height, depth);
}

void WrapperSimpleModel::CreateSphere(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	SimpleModel* self = get_self<SimpleModel>(info);
	float radius = (float)info[0].As<v8::Number>()->Value();
	int widthSegments = 32;
	int heightSegments = 16;
	if (info.Length() > 1)
	{
		widthSegments = (int)info[1].As<v8::Number>()->Value();
		if (info.Length() > 2)
		{
			heightSegments = (int)info[2].As<v8::Number>()->Value();
		}
	}
	GeometryCreator::CreateSphere(&self->geometry, radius, widthSegments, heightSegments);
}


void WrapperSimpleModel::CreatePlane(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	SimpleModel* self = get_self<SimpleModel>(info);
	float width = (float)info[0].As<v8::Number>()->Value();
	float height = (float)info[1].As<v8::Number>()->Value();	
	GeometryCreator::CreatePlane(&self->geometry, width, height);
}

void WrapperSimpleModel::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	SimpleModel* self = get_self<SimpleModel>(info);
	v8::Local<v8::Object> color = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->material.color, color);
	info.GetReturnValue().Set(color);
}

void WrapperSimpleModel::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	SimpleModel* self = get_self<SimpleModel>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->material.color, out);
	info.GetReturnValue().Set(out);
}

void WrapperSimpleModel::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	SimpleModel* self = get_self<SimpleModel>(info);
	glm::vec3 color;
	if (info[0]->IsNumber())
	{
		color.x = (float)info[0].As<v8::Number>()->Value();
		color.y = (float)info[1].As<v8::Number>()->Value();
		color.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, color);
	}
	self->set_color(color);
}

void WrapperSimpleModel::SetColorTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	SimpleModel* self = get_self<SimpleModel>(info);
	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();
	Image* image = (Image*)v8::Local<v8::External>::Cast(holder_image->GetInternalField(0))->Value();
	if (image != nullptr)
	{
		self->texture.load_memory_bgr(image->width(), image->height(), image->data(), true);
	}
}


void WrapperSimpleModel::GetMetalness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	SimpleModel* self = get_self<SimpleModel>(info);
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)self->material.metallicFactor);
	info.GetReturnValue().Set(ret);
}

void WrapperSimpleModel::SetMetalness(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	SimpleModel* self = get_self<SimpleModel>(info);
	self->set_metalness((float)value.As<v8::Number>()->Value());
}


void WrapperSimpleModel::GetRoughness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	SimpleModel* self = get_self<SimpleModel>(info);
	v8::Local<v8::Number> ret = v8::Number::New(info.GetIsolate(), (double)self->material.roughnessFactor);
	info.GetReturnValue().Set(ret);
}

void WrapperSimpleModel::SetRoughness(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	SimpleModel* self = get_self<SimpleModel>(info);
	self->set_roughness((float)value.As<v8::Number>()->Value());
}


void WrapperSimpleModel::SetToonShading(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	SimpleModel* self = get_self<SimpleModel>(info);
	int mode = (int)info[0].As<v8::Number>()->Value();
	float width = 1.5f;
	if (info.Length() > 1)
	{
		width = (float)info[1].As<v8::Number>()->Value();
	}
	self->set_toon_shading(mode, width);
}

