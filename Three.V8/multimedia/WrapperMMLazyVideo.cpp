#if THREE_MM

#include "WrapperUtils.hpp"
#include <MMLazyVideo.h>

#include "WrapperMMLazyVideo.h"

void WrapperMMLazyVideo::define(ClassDefinition& cls)
{
	cls.name = "MMLazyVideo";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "looping",   GetLooping, SetLooping },
		{ "width",  GetWidth },
		{ "height",  GetHeight },
		{ "isPlaying",  IsPlaying },
		{ "duration",  GetDuration },
		{ "position",  GetPosition },
	};
	cls.methods = {
		{"updateTexture", UpdateTexture },
		{"play", Play },
		{"pause", Pause },
		{"setPosition", SetPosition },
	};
}

void* WrapperMMLazyVideo::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	double speed = 1.0;
	if (info.Length() > 1)
	{
		lctx.jnum_to_num(info[1], speed);
	}

	return new MMLazyVideo(filename.c_str(), speed);
}

void WrapperMMLazyVideo::dtor(void* ptr, GameContext* ctx)
{
	delete (MMLazyVideo*)ptr;
}

void WrapperMMLazyVideo::UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	self->update_texture();
}

void WrapperMMLazyVideo::GetLooping(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->m_is_loop));
}

void WrapperMMLazyVideo::SetLooping(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	self->m_is_loop = value.As<v8::Boolean>()->Value();
}


void WrapperMMLazyVideo::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperMMLazyVideo::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}

void WrapperMMLazyVideo::IsPlaying(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->is_playing()));
}

void WrapperMMLazyVideo::GetDuration(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->get_total_duration_s()));
}

void WrapperMMLazyVideo::GetPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->get_current_pos_s()));
}

void WrapperMMLazyVideo::Play(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	self->play();
}

void WrapperMMLazyVideo::Pause(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	self->pause();
}

void WrapperMMLazyVideo::SetPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMLazyVideo* self = lctx.self<MMLazyVideo>();
	double pos;
	lctx.jnum_to_num(info[0], pos);
	self->set_pos_s(pos);
}

#endif