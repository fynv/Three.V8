#pragma once

#include "WrapperUtils.hpp"
#include <core/Object3D.h>
#include <core/BoundingVolumeHierarchy.h>

class WrappeBoundingVolumeHierarchy
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void Dispose(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void Intersect(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrappeBoundingVolumeHierarchy::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(1);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, Dispose));
	templ->InstanceTemplate()->Set(isolate, "intersect", v8::FunctionTemplate::New(isolate, Intersect));

	return templ;
}


void WrappeBoundingVolumeHierarchy::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8::Local<v8::Array> objects = info[0].As<v8::Array>();
	unsigned num_objects = objects->Length();

	std::vector<Object3D*> p_objs(num_objects);

	for (unsigned i = 0; i < num_objects; i++)
	{
		v8::Local<v8::Object> holder = objects->Get(context, i).ToLocalChecked().As<v8::Object>();
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(holder->GetInternalField(0));
		p_objs[i] = (Object3D*)wrap->Value();
	}

	BoundingVolumeHierarchy* self = new BoundingVolumeHierarchy(p_objs);
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
}

void WrappeBoundingVolumeHierarchy::Dispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	BoundingVolumeHierarchy* self = get_self<BoundingVolumeHierarchy>(info);
	delete self;
}

void WrappeBoundingVolumeHierarchy::Intersect(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8::Local<v8::Object> ray = info[0].As<v8::Object>();
	v8::Local<v8::Object> jorigin = ray->Get(context, v8::String::NewFromUtf8(isolate, "origin").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
	v8::Local<v8::Object> jdirection = ray->Get(context, v8::String::NewFromUtf8(isolate, "direction").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
	glm::vec3 origin, direction;
	jvec3_to_vec3(isolate, jorigin, origin);
	jvec3_to_vec3(isolate, jdirection, direction);
	float near = 0.0f;
	if (ray->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "near").ToLocalChecked()).ToChecked())
	{
		near = (float)ray->Get(context, v8::String::NewFromUtf8(isolate, "near").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}
	float far = std::numeric_limits<float>::max();
	if (ray->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "far").ToLocalChecked()).ToChecked())
	{
		far = (float)ray->Get(context, v8::String::NewFromUtf8(isolate, "far").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}

	BoundingVolumeHierarchy* self = get_self<BoundingVolumeHierarchy>(info);
	bvh::Ray<float> bvh_ray = {
		bvh::Vector3<float>(origin.x, origin.y, origin.z),
		bvh::Vector3<float>(direction.x, direction.y, direction.z),
		near,
		far
	};

	auto intersection = self->intersect(bvh_ray);

	v8::Local<v8::Object> ret = v8::Object::New(isolate);
	if (intersection->object == nullptr)
	{		
		ret->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::Null(isolate));
	}
	else
	{
		v8::Local<v8::String> name = v8::String::NewFromUtf8(isolate, intersection->object->name.c_str()).ToLocalChecked();
		ret->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), name);
	}
	ret->Set(context, v8::String::NewFromUtf8(isolate, "distance").ToLocalChecked(), v8::Number::New(isolate, (double)intersection->distance()));
	info.GetReturnValue().Set(ret);
}

