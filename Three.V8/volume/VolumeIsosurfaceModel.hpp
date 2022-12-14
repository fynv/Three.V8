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
};


v8::Local<v8::FunctionTemplate> WrapperVolumeIsosurfaceModel::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "isovalue").ToLocalChecked(), GetIsovalue, SetIsovalue);
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
