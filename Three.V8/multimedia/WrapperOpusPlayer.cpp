#if THREE_MM

#include "WrapperUtils.hpp"
#include <OpusPlayer.h>

#include "WrapperOpusPlayer.h"

void WrapperOpusPlayer::define(ClassDefinition& cls)
{
	cls.name = "OpusPlayer";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.methods = {
		{"addPacket", AddPacket },		
	};
}

void* WrapperOpusPlayer::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	int id_device = -1;
	if (info.Length() > 0)
	{
		lctx.jnum_to_num(info[0], id_device);
	}

	return new OpusPlayer(id_device);
}

void WrapperOpusPlayer::dtor(void* ptr, GameContext* ctx)
{
	delete (OpusPlayer*)ptr;
}

void WrapperOpusPlayer::AddPacket(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	OpusPlayer* self = lctx.self<OpusPlayer>();

	v8::Local<v8::ArrayBuffer> arr = info[0].As<v8::ArrayBuffer>();
	size_t size = arr->ByteLength();
	const uint8_t* data = (const uint8_t*)arr->GetBackingStore()->Data();
	self->AddPacket(size, data);
}

#endif
