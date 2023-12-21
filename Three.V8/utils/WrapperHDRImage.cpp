#include "WrapperUtils.hpp"
#include <utils/HDRImage.h>
#include "WrapperHDRImage.h"

void WrapperHDRImage::define(ClassDefinition& cls)
{
	cls.name = "HDRImage";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "width",  GetWidth },
		{ "height",  GetHeight },
	};

}

void* WrapperHDRImage::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HDRImage* self = nullptr;
	if (info.Length() > 1)
	{
		int width, height;
		lctx.jnum_to_num(info[0], width);
		lctx.jnum_to_num(info[1], height);
		self = new HDRImage(width, height);
	}
	else
	{
		self = new HDRImage();
	}
	return self;
}

void WrapperHDRImage::dtor(void* ptr, GameContext* ctx)
{
	delete (HDRImage*)ptr;
}

void WrapperHDRImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HDRImage* self = lctx.self<HDRImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperHDRImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HDRImage* self = lctx.self<HDRImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}

void WrapperHDRCubeImage::define(ClassDefinition& cls)
{
	cls.name = "HDRCubeImage";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "width",  GetWidth },
		{ "height",  GetHeight },
	};
}

void* WrapperHDRCubeImage::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new HDRCubeImage;
}

void WrapperHDRCubeImage::dtor(void* ptr, GameContext* ctx)
{
	delete (HDRCubeImage*)ptr;
}


void WrapperHDRCubeImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HDRCubeImage* self = lctx.self<HDRCubeImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->images[0].width()));
}

void WrapperHDRCubeImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HDRCubeImage* self = lctx.self<HDRCubeImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->images[0].height()));
}
