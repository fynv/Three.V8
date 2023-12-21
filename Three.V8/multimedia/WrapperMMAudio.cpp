#if THREE_MM

#include "WrapperUtils.hpp"
#include <MMPlayer.h>

#include "WrapperMMAudio.h"


void WrapperMMAudio::define(ClassDefinition& cls)
{
	cls.name = "MMAudio";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "looping",  GetLooping, SetLooping },
		{ "isPlaying",  IsPlaying },
		{ "duration",  GetDuration },
		{ "position",  GetPosition },
	};
	cls.methods = {
		{"checkEof", CheckEof },
		{"play", Play },
		{"pause", Pause },
		{"setPosition", SetPosition },
		{"setAudioDevice", SetAudioDevice },
	};
}


void* WrapperMMAudio::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
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
		v8::Local<v8::Value> holder_http = lctx.get_global("http");		
		HttpClient* http = lctx.jobj_to_obj<HttpClient>(holder_http);
		self = new MMAudio(http, filename.c_str(), device, speed);
	}
	else
	{
		self = new MMAudio(filename.c_str(), device, speed);
	}

	return self;
}

void WrapperMMAudio::dtor(void* ptr, GameContext* ctx)
{
	delete (MMAudio*)ptr;
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


#endif
