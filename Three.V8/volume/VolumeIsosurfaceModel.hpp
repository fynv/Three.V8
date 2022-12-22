#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include "volume/VolumeData.h"
#include "volume/VolumeIsosurfaceModel.h"

class WrapperVolumeIsosurfaceModel
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetIsovalue(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetIsovalue(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetMetalness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetMetalness(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetRoughness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetRoughness(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};


v8::Local<v8::FunctionTemplate> WrapperVolumeIsosurfaceModel::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "isovalue").ToLocalChecked(), GetIsovalue, SetIsovalue);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "color").ToLocalChecked(), GetColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getColor", v8::FunctionTemplate::New(isolate, GetColor));
	templ->InstanceTemplate()->Set(isolate, "setColor", v8::FunctionTemplate::New(isolate, SetColor));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "metalness").ToLocalChecked(), GetMetalness, SetMetalness);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "roughness").ToLocalChecked(), GetRoughness, SetRoughness);

	return templ;
}

void WrapperVolumeIsosurfaceModel::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	lctx.set_property(info.Holder(), "_data", info[0]);

	VolumeData* data = lctx.jobj_to_obj<VolumeData>(info[0]);
	VolumeIsosurfaceModel* self = new VolumeIsosurfaceModel(data);	
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), WrapperObject3D::dtor);
}



void WrapperVolumeIsosurfaceModel::GetIsovalue(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->m_isovalue));
}

void WrapperVolumeIsosurfaceModel::SetIsovalue(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	lctx.jnum_to_num<float>(value, self->m_isovalue);
}

void WrapperVolumeIsosurfaceModel::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	v8::Local<v8::Object> color = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->m_material.color, color);
	info.GetReturnValue().Set(color);
}

void WrapperVolumeIsosurfaceModel::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	lctx.vec3_to_jvec3(self->m_material.color, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperVolumeIsosurfaceModel::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	glm::vec3 color;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], color.x);
		lctx.jnum_to_num(info[1], color.y);
		lctx.jnum_to_num(info[2], color.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], color);
	}
	self->set_color(color);
}

void WrapperVolumeIsosurfaceModel::GetMetalness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->m_material.metallicFactor));
}

void WrapperVolumeIsosurfaceModel::SetMetalness(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	float metalness;
	lctx.jnum_to_num(value, metalness);
	self->set_metalness(metalness);
}


void WrapperVolumeIsosurfaceModel::GetRoughness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->m_material.roughnessFactor));
}

void WrapperVolumeIsosurfaceModel::SetRoughness(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	float roughness;
	lctx.jnum_to_num(value, roughness);
	self->set_roughness(roughness);
}

