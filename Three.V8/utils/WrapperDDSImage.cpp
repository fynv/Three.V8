#include "WrapperUtils.hpp"
#include <utils/DDSImage.h>
#include "WrapperDDSImage.h"

void WrapperDDSImage::define(ClassDefinition& cls)
{
	cls.name = "DDSImage";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "width",  GetWidth },
		{ "height",  GetHeight },
	};

}

void* WrapperDDSImage::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new DDSImage;
}

void WrapperDDSImage::dtor(void* ptr, GameContext* ctx)
{
	delete (DDSImage*)ptr;
}

void WrapperDDSImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DDSImage* self = lctx.self<DDSImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperDDSImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DDSImage* self = lctx.self<DDSImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}

