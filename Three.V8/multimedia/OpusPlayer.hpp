#pragma once

#include "WrapperUtils.hpp"
#include <OpusPlayer.h>

class WrapperOpusPlayer
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);	
	static void AddPacket(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperOpusPlayer::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "addPacket", v8::FunctionTemplate::New(isolate, AddPacket));
	return templ;
}


void WrapperOpusPlayer::dtor(void* ptr, GameContext* ctx)
{
	delete (OpusPlayer*)ptr;
}

void WrapperOpusPlayer::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	int id_device = -1;
	if (info.Length() > 0)
	{
		lctx.jnum_to_num(info[0], id_device);
	}

	OpusPlayer* self = new OpusPlayer(id_device);
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}


void WrapperOpusPlayer::AddPacket(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	OpusPlayer* self = lctx.self<OpusPlayer>();

	v8::Local<v8::ArrayBuffer> arr = info[0].As<v8::ArrayBuffer>();
	size_t size = arr->ByteLength();
	const uint8_t* data = (const uint8_t *)arr->GetBackingStore()->Data();
	self->AddPacket(size, data);

}