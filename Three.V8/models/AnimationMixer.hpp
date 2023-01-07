#pragma once

#include "WrapperUtils.hpp"
#include <models/AnimationMixer.h>


class WrapperAnimationMixer
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);

	static void GetAnimations(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetAnimations(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void AddAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void AddAnimations(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void StartAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void StopAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetCurrentPlaying(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetWeights(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetFrame(const v8::FunctionCallbackInfo<v8::Value>& info);
	
};

v8::Local<v8::FunctionTemplate> WrapperAnimationMixer::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "animations").ToLocalChecked(), GetAnimations, 0);
	templ->InstanceTemplate()->Set(isolate, "getAnimation", v8::FunctionTemplate::New(isolate, GetAnimation));
	templ->InstanceTemplate()->Set(isolate, "getAnimations", v8::FunctionTemplate::New(isolate, GetAnimations));

	templ->InstanceTemplate()->Set(isolate, "addAnimation", v8::FunctionTemplate::New(isolate, AddAnimation));
	templ->InstanceTemplate()->Set(isolate, "addAnimations", v8::FunctionTemplate::New(isolate, AddAnimations));

	templ->InstanceTemplate()->Set(isolate, "startAnimation", v8::FunctionTemplate::New(isolate, StartAnimation));
	templ->InstanceTemplate()->Set(isolate, "stopAnimation", v8::FunctionTemplate::New(isolate, StopAnimation));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "currentPlaying").ToLocalChecked(), GetCurrentPlaying, 0);
	templ->InstanceTemplate()->Set(isolate, "setWeights", v8::FunctionTemplate::New(isolate, SetWeights));

	templ->InstanceTemplate()->Set(isolate, "getFrame", v8::FunctionTemplate::New(isolate, GetFrame));
	
	return templ;
}


void WrapperAnimationMixer::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = new AnimationMixer();
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), WrapperAnimationMixer::dtor);
}

void WrapperAnimationMixer::dtor(void* ptr, GameContext* ctx)
{
	delete (AnimationMixer*)ptr;
}



void WrapperAnimationMixer::GetAnimations(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();

	v8::Local<v8::Array> janims = v8::Array::New(lctx.isolate, (int)self->m_animations.size());

	for (size_t i = 0; i < self->m_animations.size(); i++)
	{
		AnimationClip& anim = self->m_animations[i];

		v8::Local<v8::Object> janim = v8::Object::New(lctx.isolate);
		lctx.set_property(janim, "name", lctx.str_to_jstr(anim.name.c_str()));
		lctx.set_property(janim, "duration", lctx.num_to_jnum(anim.duration));

		v8::Local<v8::Array> jmorphs = v8::Array::New(lctx.isolate, (int)anim.morphs.size());
		for (size_t j = 0; j < anim.morphs.size(); j++)
		{
			const MorphTrack& morph = anim.morphs[j];

			v8::Local<v8::Object> jmorph = v8::Object::New(lctx.isolate);
			lctx.set_property(jmorph, "name", lctx.str_to_jstr(morph.name.c_str()));
			lctx.set_property(jmorph, "targets", lctx.num_to_jnum(morph.num_targets));
			lctx.set_property(jmorph, "frames", lctx.num_to_jnum(morph.times.size()));
			jmorphs->Set(lctx.context, (unsigned)j, jmorph);
		}
		lctx.set_property(janim, "morphs", jmorphs);
		janims->Set(lctx.context, (unsigned)i, janim);
	}

	info.GetReturnValue().Set(janims);
}

void WrapperAnimationMixer::GetAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();

	std::string name = lctx.jstr_to_str(info[0]);
	auto iter = self->m_animation_dict.find(name);
	if (iter != self->m_animation_dict.end())
	{
		AnimationClip& anim = self->m_animations[iter->second];
		v8::Local<v8::Object> janim = v8::Object::New(lctx.isolate);
		lctx.anim_to_janim(anim, janim);
		info.GetReturnValue().Set(janim);
	}
}

void WrapperAnimationMixer::GetAnimations(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();

	v8::Local<v8::Array> janims = v8::Array::New(lctx.isolate, (int)self->m_animations.size());

	for (size_t i = 0; i < self->m_animations.size(); i++)
	{
		AnimationClip& anim = self->m_animations[i];
		v8::Local<v8::Object> janim = v8::Object::New(lctx.isolate);
		lctx.anim_to_janim(anim, janim);
		janims->Set(lctx.context, (unsigned)i, janim);
	}

	info.GetReturnValue().Set(janims);
}

void WrapperAnimationMixer::AddAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();

	AnimationClip anim;
	lctx.janim_to_anim(info[0], anim);
	self->addAnimation(anim);
}

void WrapperAnimationMixer::AddAnimations(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();

	v8::Local<v8::Array> jAnims = info[0].As<v8::Array>();
	for (unsigned i = 0; i < jAnims->Length(); i++)
	{
		v8::Local<v8::Value> jAnim = jAnims->Get(lctx.context, i).ToLocalChecked();
		AnimationClip anim;
		lctx.janim_to_anim(jAnim, anim);
		self->addAnimation(anim);
	}
}

void WrapperAnimationMixer::StartAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();
	self->startAnimation(lctx.jstr_to_str(info[0]).c_str());
}

void WrapperAnimationMixer::StopAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();
	self->stopAnimation(lctx.jstr_to_str(info[0]).c_str());
}

void WrapperAnimationMixer::GetCurrentPlaying(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();
	v8::Local<v8::Array> arr = v8::Array::New(lctx.isolate, self->m_current_playing.size());
	for (size_t i = 0; i < self->m_current_playing.size(); i++)
	{
		AnimationMixer::PlayBack& playback = self->m_current_playing[i];
		AnimationClip& clip = self->m_animations[playback.id_anim];
		v8::Local<v8::String> name = lctx.str_to_jstr(clip.name.c_str());
		v8::Local<v8::Number> weight = lctx.num_to_jnum(playback.weight);
		v8::Local<v8::Object> play = v8::Object::New(lctx.isolate);
		lctx.set_property(play, "name", name);
		lctx.set_property(play, "weight", weight);
		arr->Set(lctx.context, i, play);
	}
	info.GetReturnValue().Set(arr);
}


void WrapperAnimationMixer::SetWeights(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();
	v8::Local<v8::Array> weights = info[0].As<v8::Array>();
	for (unsigned i = 0; i < weights->Length(); i++)
	{
		if (i >= self->m_current_playing.size()) break;
		float w;
		lctx.jnum_to_num(weights->Get(lctx.context, i).ToLocalChecked(), w);
		self->m_current_playing[i].weight = w;
	}

	for (size_t i = 0; i < self->m_current_playing.size(); i++)
	{
		if (self->m_current_playing[i].weight <= 0.0f)
		{
			self->m_current_playing.erase(self->m_current_playing.begin() + i);
			i--;
		}
	}

}

void WrapperAnimationMixer::GetFrame(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	AnimationMixer* self = lctx.self<AnimationMixer>();
	AnimationFrame frame;
	self->getFrame(frame);
	v8::Local<v8::Object> jframe = v8::Object::New(lctx.isolate);	
	lctx.frame_to_jframe(frame, jframe);
	info.GetReturnValue().Set(jframe);
}
