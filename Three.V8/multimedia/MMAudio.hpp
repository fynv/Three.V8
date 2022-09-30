#pragma once

#include "WrapperUtils.hpp"
#include <MMPlayer.h>

class WrapperMMAudio
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);	
	static void CheckEof(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetLooping(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetLooping(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
	static void IsPlaying(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetDuration(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Play(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Pause(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetPosition(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetAudioDevice(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperMMAudio::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));	
	templ->InstanceTemplate()->Set(isolate, "checkEof", v8::FunctionTemplate::New(isolate, CheckEof));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "looping").ToLocalChecked(), GetLooping, SetLooping);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "isPlaying").ToLocalChecked(), IsPlaying, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "duration").ToLocalChecked(), GetDuration, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "position").ToLocalChecked(), GetPosition, 0);
	templ->InstanceTemplate()->Set(isolate, "play", v8::FunctionTemplate::New(isolate, Play));
	templ->InstanceTemplate()->Set(isolate, "pause", v8::FunctionTemplate::New(isolate, Pause));
	templ->InstanceTemplate()->Set(isolate, "setPosition", v8::FunctionTemplate::New(isolate, SetPosition));
	templ->InstanceTemplate()->Set(isolate, "setAudioDevice", v8::FunctionTemplate::New(isolate, SetAudioDevice));
	return templ;
}


void WrapperMMAudio::dtor(void* ptr, GameContext* ctx)
{
	delete (MMAudio*)ptr;
}

void WrapperMMAudio::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	bool http = false;
	if (filename.length() > 4 && filename.substr(0, 4) == "http")
	{
		http = true;
	}

	int device = -1;
	int speed = 1;	
	if (info.Length() > 1)
	{
		lctx.jnum_to_num(info[1], device);

		if (info.Length() > 2)
		{
			lctx.jnum_to_num(info[2], speed);
		}
	}

	MMAudio* self;
	if (http)
	{
		GamePlayer* player = lctx.player();
		HttpClient* http = lctx.ctx()->GetHttpClient();
		self = new MMAudio(http, filename.c_str(), device, speed);
	}
	else
	{
		self = new MMAudio(filename.c_str(), device, speed);
	}
	
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}


void WrapperMMAudio::CheckEof(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	self->check_eof();
}


void WrapperMMAudio::GetLooping(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->m_is_loop));
}

void WrapperMMAudio::SetLooping(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	self->m_is_loop = value.As<v8::Boolean>()->Value();
}


void WrapperMMAudio::IsPlaying(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->is_playing()));
}


void WrapperMMAudio::GetDuration(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->get_total_duration_s()));
}

void WrapperMMAudio::GetPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->get_current_pos_s()));
}

void WrapperMMAudio::Play(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	self->play();
}

void WrapperMMAudio::Pause(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	self->pause();
}

void WrapperMMAudio::SetPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	double pos;
	lctx.jnum_to_num(info[0], pos);
	self->set_pos_s(pos);
}


void WrapperMMAudio::SetAudioDevice(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMAudio* self = lctx.self<MMAudio>();
	int id_device;
	lctx.jnum_to_num(info[0], id_device);
	self->set_audio_device(id_device);
}


