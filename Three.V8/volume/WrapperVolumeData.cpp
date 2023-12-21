#include "WrapperUtils.hpp"
#include <volume/VolumeData.h>

#include "WrapperVolumeData.h"

void WrapperVolumeData::define(ClassDefinition& cls)
{
	cls.name = "VolumeData";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "width",  GetWidth },
		{ "height",  GetHeight },
		{ "depth",  GetDepth },
		{ "spacing",  GetSpacing },
		{ "bytesPerPixel",  GetBytesPerPixel },
	};

}

void* WrapperVolumeData::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new VolumeData;
}

void WrapperVolumeData::dtor(void* ptr, GameContext* ctx)
{
	delete (VolumeData*)ptr;
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
