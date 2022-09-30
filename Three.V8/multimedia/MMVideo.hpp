#pragma once

#include "WrapperUtils.hpp"
#include <MMPlayer.h>

class WrapperMMVideo
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
	static void SetAudioDevice(const v8::FunctionCallbackInfo<v8::Value>& info);
};

v8::Local<v8::FunctionTemplate> WrapperMMVideo::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
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
	templ->InstanceTemplate()->Set(isolate, "setAudioDevice", v8::FunctionTemplate::New(isolate, SetAudioDevice));
	return templ;
}

void WrapperMMVideo::dtor(void* ptr, GameContext* ctx)
{
	delete (MMVideo*)ptr;
}

void WrapperMMVideo::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	bool http = false;
	if (filename.length() > 4 && filename.substr(0, 4) == "http")
	{
		http = true;
	}

	bool play_audio = true;
	int device = -1;
	int speed = 1;
	if (info.Length() > 1)
	{
		play_audio = info[1].As<v8::Boolean>()->Value();
		if (info.Length() > 2)
		{
			lctx.jnum_to_num(info[2], device);

			if (info.Length() > 3)
			{
				lctx.jnum_to_num(info[3], speed);
			}
		}
	}

	MMVideo* self;
	if (http)
	{
		GamePlayer* player = lctx.player();
		HttpClient* http = lctx.ctx()->GetHttpClient();
		self = new MMVideo(http, filename.c_str(), play_audio, device, speed);
	}
	else
	{
		self = new MMVideo(filename.c_str(), play_audio, device, speed);
	}
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}

void WrapperMMVideo::UpdateTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	self->update_texture();
}

void WrapperMMVideo::GetLooping(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->m_is_loop));
}

void WrapperMMVideo::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperMMVideo::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}

void WrapperMMVideo::SetLooping(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	self->m_is_loop = value.As<v8::Boolean>()->Value();
}


void WrapperMMVideo::IsPlaying(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->is_playing()));
}


void WrapperMMVideo::GetDuration(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->get_total_duration_s()));
}

void WrapperMMVideo::GetPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->get_current_pos_s()));
}

void WrapperMMVideo::Play(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	self->play();
}

void WrapperMMVideo::Pause(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	self->pause();
}

void WrapperMMVideo::SetPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	double pos;
	lctx.jnum_to_num(info[0], pos);
	self->set_pos_s(pos);
}


void WrapperMMVideo::SetAudioDevice(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	MMVideo* self = lctx.self<MMVideo>();
	int id_device;
	lctx.jnum_to_num(info[0], id_device);
	self->set_audio_device(id_device);
}


