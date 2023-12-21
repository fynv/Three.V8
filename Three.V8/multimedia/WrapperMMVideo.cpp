#if THREE_MM

#include "WrapperUtils.hpp"
#include <MMPlayer.h>

#include "WrapperMMVideo.h"

void WrapperMMVideo::define(ClassDefinition& cls)
{
	cls.name = "MMVideo";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "looping",  GetLooping, SetLooping },
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
		{"setAudioDevice", SetAudioDevice },
	};
}


void* WrapperMMVideo::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
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
		v8::Local<v8::Value> holder_http = lctx.get_global("http");
		HttpClient* http = lctx.jobj_to_obj<HttpClient>(holder_http);
		self = new MMVideo(http, filename.c_str(), play_audio, device, speed);
	}
	else
	{
		self = new MMVideo(filename.c_str(), play_audio, device, speed);
	}

	return self;
}

void WrapperMMVideo::dtor(void* ptr, GameContext* ctx)
{
	delete (MMVideo*)ptr;
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


#endif

