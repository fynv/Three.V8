#if THREE_MM

#include "WrapperUtils.hpp"
#include <AVCPlayer.h>

#include "WrapperAVCPlayer.h"


void WrapperAVCPlayer::define(ClassDefinition& cls)
{
	cls.name = "AVCPlayer";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "width",  GetWidth },
		{ "height",  GetHeight },
	};
	cls.methods = {
		{"addPacket", AddPacket },
		{"updateTexture", UpdateTexture },
	};
}

void* WrapperAVCPlayer::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new AVCPlayer;
}

void WrapperAVCPlayer::dtor(void* ptr, GameContext* ctx)
{
	delete (AVCPlayer*)ptr;
}

void WrapperAVCPlayer::AddPacket(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AVCPlayer* self = lctx.self<AVCPlayer>();

	v8::Local<v8::ArrayBuffer> arr = info[0].As<v8::ArrayBuffer>();
	size_t size = arr->ByteLength();
	const uint8_t* data = (const uint8_t*)arr->GetBackingStore()->Data();
	self->AddPacket(size, data);

}

void WrapperAVCPlayer::UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AVCPlayer* self = lctx.self<AVCPlayer>();
	self->update_texture();
}

void WrapperAVCPlayer::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AVCPlayer* self = lctx.self<AVCPlayer>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperAVCPlayer::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AVCPlayer* self = lctx.self<AVCPlayer>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}



#endif

