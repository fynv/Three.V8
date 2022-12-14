#pragma once

#include "WrapperUtils.hpp"
#include <volume/VolumeData.h>

class WrapperVolumeData
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetDepth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSpacing(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetBytesPerPixel(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperVolumeData::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);

	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "depth").ToLocalChecked(), GetDepth, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "spacing").ToLocalChecked(), GetSpacing, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "bytesPerPixel").ToLocalChecked(), GetBytesPerPixel, 0);

	return templ;
}


void WrapperVolumeData::dtor(void* ptr, GameContext* ctx)
{
	delete (VolumeData*)ptr;
}

void WrapperVolumeData::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeData* self = new VolumeData();
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}


void WrapperVolumeData::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeData* self = lctx.self<VolumeData>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->size.x));
}

void WrapperVolumeData::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeData* self = lctx.self<VolumeData>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->size.y));
}

void WrapperVolumeData::GetDepth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeData* self = lctx.self<VolumeData>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->size.z));
}

void WrapperVolumeData::GetSpacing(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeData* self = lctx.self<VolumeData>();

	v8::Local<v8::Object> obj = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->spacing, obj);
	info.GetReturnValue().Set(obj);
}


void WrapperVolumeData::GetBytesPerPixel(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeData* self = lctx.self<VolumeData>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->bytes_per_pixel));
}
