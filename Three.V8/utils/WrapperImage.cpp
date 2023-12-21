#include "WrapperUtils.hpp"
#include <utils/Image.h>
#include "WrapperImage.h"

void WrapperImage::define(ClassDefinition& cls)
{
	cls.name = "Image";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "width",  GetWidth },
		{ "height",  GetHeight },
	};
	
}

void* WrapperImage::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Image* self = nullptr;
	if (info.Length() > 1)
	{
		int width, height;
		lctx.jnum_to_num(info[0], width);
		lctx.jnum_to_num(info[1], height);
		self = new Image(width, height);
	}
	else
	{
		self = new Image();
	}
	return self;
}

void WrapperImage::dtor(void* ptr, GameContext* ctx)
{
	delete (Image*)ptr;
}

void WrapperImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Image* self = lctx.self<Image>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Image* self = lctx.self<Image>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}

void WrapperCubeImage::define(ClassDefinition& cls)
{
	cls.name = "CubeImage";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "width",  GetWidth },
		{ "height",  GetHeight },
	};

}

void* WrapperCubeImage::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new CubeImage;
}

void WrapperCubeImage::dtor(void* ptr, GameContext* ctx)
{
	delete (CubeImage*)ptr;
}

void WrapperCubeImage::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeImage* self = lctx.self<CubeImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->images[0].width()));
}

void WrapperCubeImage::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeImage* self = lctx.self<CubeImage>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->images[0].height()));
}
