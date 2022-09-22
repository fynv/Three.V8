#pragma once

#include "WrapperUtils.hpp"
#include <MMLazyVideo.h>

class WrapperMMLazyVideo
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);
	static void UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetLooping(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetLooping(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void IsPlaying(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetDuration(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Play(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Pause(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetPosition(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperMMLazyVideo::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "updateTexture", v8::FunctionTemplate::New(isolate, UpdateTexture));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "looping").ToLocalChecked(), GetLooping, SetLooping);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "isPlaying").ToLocalChecked(), IsPlaying, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "duration").ToLocalChecked(), GetDuration, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "position").ToLocalChecked(), GetPosition, 0);
	templ->InstanceTemplate()->Set(isolate, "play", v8::FunctionTemplate::New(isolate, Play));
	templ->InstanceTemplate()->Set(isolate, "pause", v8::FunctionTemplate::New(isolate, Pause));
	templ->InstanceTemplate()->Set(isolate, "setPosition", v8::FunctionTemplate::New(isolate, SetPosition));
	return templ;
}

void WrapperMMLazyVideo::dtor(void* ptr, GameContext* ctx)
{
	delete (MMLazyVideo*)ptr;
}


void WrapperMMLazyVideo::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	double speed = 1.0;
	if (info.Length() > 1)
	{
		lctx.jnum_to_num(info[1], speed);
	}

	MMLazyVideo* self = new MMLazyVideo(filename.c_str(), speed);
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
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



