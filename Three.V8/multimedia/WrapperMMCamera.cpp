#if THREE_MM

#include "WrapperUtils.hpp"
#include <MMCamera.h>

#include "WrapperMMCamera.h"

void WrapperMMCamera::define(ClassDefinition& cls)
{
	cls.name = "MMCamera";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "width",  GetWidth },
		{ "height",  GetHeight },
	};
	cls.methods = {
		{"updateTexture", UpdateTexture },	
	};
}

void* WrapperMMCamera::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	int idx = 0;
	if (info.Length() > 0)
	{
		lctx.jnum_to_num(info[0], idx);
	}
	return new MMCamera(idx);
}

void WrapperMMCamera::dtor(void* ptr, GameContext* ctx)
{
	delete (MMCamera*)ptr;
}

void WrapperMMCamera::UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMCamera* self = lctx.self<MMCamera>();
	self->update_texture();
}


void WrapperMMCamera::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMCamera* self = lctx.self<MMCamera>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperMMCamera::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMCamera* self = lctx.self<MMCamera>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}




#endif

