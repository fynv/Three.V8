#pragma once

#include <v8.h>
#include <glm.hpp>
#include <gtx/quaternion.hpp>
#include <models/Animation.h>

template<class T, class InfoType>
inline T* get_self(const InfoType& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(holder->GetInternalField(0));
	T* self = (T*)wrap->Value();
	return self;
}

inline void vec3_to_jvec3(v8::Isolate* isolate, const glm::vec3& vec, v8::Local<v8::Object> jvec)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	jvec->Set(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), v8::Number::New(isolate, (double)vec.x));
	jvec->Set(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), v8::Number::New(isolate, (double)vec.y));
	jvec->Set(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked(), v8::Number::New(isolate, (double)vec.z));
}

inline void jvec3_to_vec3(v8::Isolate* isolate, v8::Local<v8::Object> jvec, glm::vec3& vec)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	vec.x = (float)jvec->Get(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	vec.y = (float)jvec->Get(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	vec.z = (float)jvec->Get(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
}

inline void quat_to_jquat(v8::Isolate* isolate, const glm::quat& quat, v8::Local<v8::Object> jquat)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	jquat->Set(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), v8::Number::New(isolate, (double)quat.x));
	jquat->Set(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), v8::Number::New(isolate, (double)quat.y));
	jquat->Set(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked(), v8::Number::New(isolate, (double)quat.z));
	jquat->Set(context, v8::String::NewFromUtf8(isolate, "w").ToLocalChecked(), v8::Number::New(isolate, (double)quat.w));
}

inline void jquat_to_quat(v8::Isolate* isolate, v8::Local<v8::Object> jquat, glm::quat& quat)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	quat.x = (float)jquat->Get(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	quat.y = (float)jquat->Get(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	quat.z = (float)jquat->Get(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	quat.w = (float)jquat->Get(context, v8::String::NewFromUtf8(isolate, "w").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
}

inline void mat4_to_jmat4(v8::Isolate* isolate, const glm::mat4& matrix, v8::Local<v8::Object> jmatrix)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Array> elements;
	if (jmatrix->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToChecked())
	{
		elements = jmatrix->Get(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	}
	else
	{
		elements = v8::Array::New(isolate, 16);
		jmatrix->Set(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked(), elements);
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			elements->Set(context, j + i * 4, v8::Number::New(isolate, (double)matrix[i][j]));
		}
	}
}

inline void jmat4_to_mat4(v8::Isolate* isolate, v8::Local<v8::Object> jmatrix, glm::mat4& matrix)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Array> elements = jmatrix->Get(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			matrix[i][j] = (float)elements->Get(context, j + i * 4).ToLocalChecked().As<v8::Number>()->Value();
		}
	}
}

inline void jframe_to_frame(v8::Isolate* isolate, v8::Local<v8::Object> jFrame, AnimationFrame& frame)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8::Local<v8::Array> jMorphs = jFrame->Get(context, v8::String::NewFromUtf8(isolate, "morphs").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	frame.morphs.resize(jMorphs->Length());
	for (unsigned i = 0; i < jMorphs->Length(); i++)
	{
		v8::Local<v8::Object> jMorph = jMorphs->Get(context, i).ToLocalChecked().As<v8::Object>();
		v8::Local<v8::Value> jName = jMorph->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
		v8::String::Utf8Value name(isolate, jName);
		v8::Local<v8::Array> jWeights = jMorph->Get(context, v8::String::NewFromUtf8(isolate, "weights").ToLocalChecked()).ToLocalChecked().As<v8::Array>();

		MorphFrame& morph = frame.morphs[i];
		morph.name = *name;
		morph.weights.resize(jWeights->Length());
		for (unsigned j = 0; j < jWeights->Length(); j++)
		{
			morph.weights[j] = (float)jWeights->Get(context, j).ToLocalChecked().As<v8::Number>()->Value();
		}
	}
}

inline void anim_to_janim(v8::Isolate* isolate, const AnimationClip& anim, v8::Local<v8::Object> janim)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	static std::string interpolation_map[3] = { "STEP", "LINEAR", "CUBICSPLINE" };

	janim->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::String::NewFromUtf8(isolate, anim.name.c_str()).ToLocalChecked());
	janim->Set(context, v8::String::NewFromUtf8(isolate, "start").ToLocalChecked(), v8::Number::New(isolate, anim.start));
	janim->Set(context, v8::String::NewFromUtf8(isolate, "end").ToLocalChecked(), v8::Number::New(isolate, anim.end));

	v8::Local<v8::Array> jmorphs = v8::Array::New(isolate, anim.morphs.size());
	for (size_t j = 0; j < anim.morphs.size(); j++)
	{
		const MorphTrack& morph = anim.morphs[j];

		v8::Local<v8::Object> jmorph = v8::Object::New(isolate);
		jmorph->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::String::NewFromUtf8(isolate, morph.name.c_str()).ToLocalChecked());
		jmorph->Set(context, v8::String::NewFromUtf8(isolate, "targets").ToLocalChecked(), v8::Number::New(isolate, (double)morph.num_targets));

		std::string interpolation = interpolation_map[(unsigned)morph.interpolation];
		jmorph->Set(context, v8::String::NewFromUtf8(isolate, "interpolation").ToLocalChecked(), v8::String::NewFromUtf8(isolate, interpolation.c_str()).ToLocalChecked());

		v8::Local<v8::ArrayBuffer> jtimes = v8::ArrayBuffer::New(isolate, sizeof(float) * morph.times.size());
		float* p_times = (float*)jtimes->GetBackingStore()->Data();
		memcpy(p_times, morph.times.data(), sizeof(float) * morph.times.size());
		jmorph->Set(context, v8::String::NewFromUtf8(isolate, "times").ToLocalChecked(), v8::Float32Array::New(jtimes, 0, morph.times.size()));

		v8::Local<v8::ArrayBuffer> jvalues = v8::ArrayBuffer::New(isolate, sizeof(float) * morph.values.size());
		float* p_values = (float*)jvalues->GetBackingStore()->Data();
		memcpy(p_values, morph.values.data(), sizeof(float) * morph.values.size());
		jmorph->Set(context, v8::String::NewFromUtf8(isolate, "values").ToLocalChecked(), v8::Float32Array::New(jvalues, 0, morph.values.size()));

		jmorphs->Set(context, j, jmorph);
	}

	janim->Set(context, v8::String::NewFromUtf8(isolate, "morphs").ToLocalChecked(), jmorphs);
}


inline void janim_to_anim(v8::Isolate* isolate, v8::Local<v8::Object> janim, AnimationClip& anim)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8::Local<v8::Value> jName = janim->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
	v8::String::Utf8Value name(isolate, jName);
	anim.name = *name;
	anim.start = janim->Get(context, v8::String::NewFromUtf8(isolate, "start").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	anim.end = janim->Get(context, v8::String::NewFromUtf8(isolate, "end").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();

	v8::Local<v8::Array> jMorphs = janim->Get(context, v8::String::NewFromUtf8(isolate, "morphs").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	anim.morphs.resize(jMorphs->Length());
	for (unsigned i = 0; i < jMorphs->Length(); i++)
	{
		v8::Local<v8::Object> jMorph = jMorphs->Get(context, i).ToLocalChecked().As<v8::Object>();
		MorphTrack& morph = anim.morphs[i];

		v8::Local<v8::Value> jName = jMorph->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
		v8::String::Utf8Value name(isolate, jName);
		morph.name = *name;
		morph.num_targets = (int)jMorph->Get(context, v8::String::NewFromUtf8(isolate, "targets").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();

		v8::Local<v8::Value> jInterpolation = jMorph->Get(context, v8::String::NewFromUtf8(isolate, "interpolation").ToLocalChecked()).ToLocalChecked();
		v8::String::Utf8Value uInterpolation(isolate, jInterpolation);
		std::string interpolation = *uInterpolation;

		if (interpolation == "STEP")
		{
			morph.interpolation = Interpolation::STEP;
		}
		else if (interpolation == "LINEAR")
		{
			morph.interpolation = Interpolation::LINEAR;
		}
		else if (interpolation == "CUBICSPLINE")
		{
			morph.interpolation = Interpolation::CUBICSPLINE;
		}

		v8::Local<v8::Float32Array> jtimes = jMorph->Get(context, v8::String::NewFromUtf8(isolate, "times").ToLocalChecked()).ToLocalChecked().As<v8::Float32Array>();
		morph.times.resize(jtimes->Length());
		const float* p_times = (const float*)jtimes->Buffer()->GetBackingStore()->Data();
		memcpy(morph.times.data(), p_times, sizeof(float) * jtimes->Length());

		v8::Local<v8::Float32Array> jvalues = jMorph->Get(context, v8::String::NewFromUtf8(isolate, "values").ToLocalChecked()).ToLocalChecked().As<v8::Float32Array>();
		morph.values.resize(jvalues->Length());
		const float* p_values = (const float*)jvalues->Buffer()->GetBackingStore()->Data();
		memcpy(morph.values.data(), p_values, sizeof(float) * jvalues->Length());		
	}
}