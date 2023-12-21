#include "WrapperUtils.hpp"
#include <models/AnimationMixer.h>
#include "WrapperAnimationMixer.h"

void WrapperAnimationMixer::define(ClassDefinition& cls)
{
	cls.name = "AnimationMixer";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "animations",  GetAnimations },
		{ "currentPlaying",  GetCurrentPlaying },
	};
	cls.methods = {
		{"getAnimation", GetAnimation },
		{"getAnimations", GetAnimations },
		{"addAnimation", AddAnimation },
		{"addAnimations", AddAnimations },
		{"startAnimation", StartAnimation },
		{"stopAnimation", StopAnimation },
		{"setWeights", SetWeights },
		{"getFrame", GetFrame },
	};
}

void* WrapperAnimationMixer::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new AnimationMixer;
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
	int idx;
	lctx.jnum_to_num(info[0], idx);
	self->stopAnimation(idx);
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
