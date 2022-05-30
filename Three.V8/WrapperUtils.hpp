#pragma once

#include <v8.h>
#include <unordered_map>
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

template<class InfoType>
inline GameContext* get_context(const InfoType& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(global->GetInternalField(0));
	GameContext* ctx = (GameContext*)wrap->Value();
	return ctx;
}

void GeneralDispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	void* self = get_self<void>(info);
	GameContext* ctx = get_context(info);
	ctx->remove_object(self);
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

	if (jFrame->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "morphs").ToLocalChecked()).ToChecked())
	{
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

	if (jFrame->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "translations").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Array> jTransLst = jFrame->Get(context, v8::String::NewFromUtf8(isolate, "translations").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
		frame.translations.resize(jTransLst->Length());
		for (unsigned i = 0; i < jTransLst->Length(); i++)
		{
			v8::Local<v8::Object> jTransFrame = jTransLst->Get(context, i).ToLocalChecked().As<v8::Object>();
			v8::Local<v8::Value> jName = jTransFrame->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
			v8::String::Utf8Value name(isolate, jName);

			TranslationFrame& trans = frame.translations[i];
			trans.name = *name;			
			v8::Local<v8::Object> jTranslation = jTransFrame->Get(context, v8::String::NewFromUtf8(isolate, "translation").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
			jvec3_to_vec3(isolate, jTranslation, trans.translation);			
		}
	}

	if (jFrame->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "rotations").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Array> jRotLst = jFrame->Get(context, v8::String::NewFromUtf8(isolate, "rotations").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
		frame.rotations.resize(jRotLst->Length());
		for (unsigned i = 0; i < jRotLst->Length(); i++)
		{
			v8::Local<v8::Object> jRotFrame = jRotLst->Get(context, i).ToLocalChecked().As<v8::Object>();
			v8::Local<v8::Value> jName = jRotFrame->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
			v8::String::Utf8Value name(isolate, jName);

			RotationFrame& rot = frame.rotations[i];
			rot.name = *name;
			v8::Local<v8::Object> jRotation = jRotFrame->Get(context, v8::String::NewFromUtf8(isolate, "rotation").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
			jquat_to_quat(isolate, jRotation, rot.rotation);
		}
	}

	if (jFrame->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "scales").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Array> jScaleLst = jFrame->Get(context, v8::String::NewFromUtf8(isolate, "scales").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
		frame.rotations.resize(jScaleLst->Length());
		for (unsigned i = 0; i < jScaleLst->Length(); i++)
		{
			v8::Local<v8::Object> jScaleFrame = jScaleLst->Get(context, i).ToLocalChecked().As<v8::Object>();
			v8::Local<v8::Value> jName = jScaleFrame->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
			v8::String::Utf8Value name(isolate, jName);

			ScaleFrame& scale = frame.scales[i];
			scale.name = *name;
			v8::Local<v8::Object> jScale = jScaleFrame->Get(context, v8::String::NewFromUtf8(isolate, "scale").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
			jvec3_to_vec3(isolate, jScale, scale.scale);
		}
	}
}

inline void anim_to_janim(v8::Isolate* isolate, const AnimationClip& anim, v8::Local<v8::Object> janim)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	static std::string interpolation_map[3] = { "STEP", "LINEAR", "CUBICSPLINE" };

	janim->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::String::NewFromUtf8(isolate, anim.name.c_str()).ToLocalChecked());
	janim->Set(context, v8::String::NewFromUtf8(isolate, "duration").ToLocalChecked(), v8::Number::New(isolate, anim.duration));	

	size_t num_morphs = anim.morphs.size();
	if (num_morphs > 0)
	{
		v8::Local<v8::Array> jmorphs = v8::Array::New(isolate, (int)num_morphs);
		for (size_t j = 0; j < num_morphs; j++)
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
			void* p_values = jvalues->GetBackingStore()->Data();
			memcpy(p_values, morph.values.data(), sizeof(float) * morph.values.size());
			jmorph->Set(context, v8::String::NewFromUtf8(isolate, "values").ToLocalChecked(), v8::Float32Array::New(jvalues, 0, morph.values.size()));

			jmorphs->Set(context, (unsigned)j, jmorph);
		}
		janim->Set(context, v8::String::NewFromUtf8(isolate, "morphs").ToLocalChecked(), jmorphs);
	}

	size_t num_trans = anim.translations.size();
	if (num_trans > 0)
	{
		v8::Local<v8::Array> jTransLst = v8::Array::New(isolate, (int)num_trans);
		for (size_t j = 0; j < num_trans; j++)
		{
			const TranslationTrack& trans = anim.translations[j];

			v8::Local<v8::Object> jTrans = v8::Object::New(isolate);
			jTrans->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::String::NewFromUtf8(isolate, trans.name.c_str()).ToLocalChecked());

			std::string interpolation = interpolation_map[(unsigned)trans.interpolation];
			jTrans->Set(context, v8::String::NewFromUtf8(isolate, "interpolation").ToLocalChecked(), v8::String::NewFromUtf8(isolate, interpolation.c_str()).ToLocalChecked());

			v8::Local<v8::ArrayBuffer> jtimes = v8::ArrayBuffer::New(isolate, sizeof(float) * trans.times.size());
			float* p_times = (float*)jtimes->GetBackingStore()->Data();
			memcpy(p_times, trans.times.data(), sizeof(float) * trans.times.size());
			jTrans->Set(context, v8::String::NewFromUtf8(isolate, "times").ToLocalChecked(), v8::Float32Array::New(jtimes, 0, trans.times.size()));

			v8::Local<v8::ArrayBuffer> jvalues = v8::ArrayBuffer::New(isolate, sizeof(glm::vec3) * trans.values.size());
			void* p_values = jvalues->GetBackingStore()->Data();
			memcpy(p_values, trans.values.data(), sizeof(glm::vec3) * trans.values.size());
			jTrans->Set(context, v8::String::NewFromUtf8(isolate, "values").ToLocalChecked(), v8::Float32Array::New(jvalues, 0, trans.values.size()*3));

			jTransLst->Set(context, (unsigned)j, jTrans);
		}
		janim->Set(context, v8::String::NewFromUtf8(isolate, "translations").ToLocalChecked(), jTransLst);
	}

	size_t num_rots = anim.rotations.size();
	if (num_rots > 0)
	{
		v8::Local<v8::Array> jRotLst = v8::Array::New(isolate, (int)num_rots);
		for (size_t j = 0; j < num_rots; j++)
		{
			const RotationTrack& rot = anim.rotations[j];

			v8::Local<v8::Object> jRot = v8::Object::New(isolate);
			jRot->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::String::NewFromUtf8(isolate, rot.name.c_str()).ToLocalChecked());

			std::string interpolation = interpolation_map[(unsigned)rot.interpolation];
			jRot->Set(context, v8::String::NewFromUtf8(isolate, "interpolation").ToLocalChecked(), v8::String::NewFromUtf8(isolate, interpolation.c_str()).ToLocalChecked());

			v8::Local<v8::ArrayBuffer> jtimes = v8::ArrayBuffer::New(isolate, sizeof(float) * rot.times.size());
			float* p_times = (float*)jtimes->GetBackingStore()->Data();
			memcpy(p_times, rot.times.data(), sizeof(float) * rot.times.size());
			jRot->Set(context, v8::String::NewFromUtf8(isolate, "times").ToLocalChecked(), v8::Float32Array::New(jtimes, 0, rot.times.size()));

			v8::Local<v8::ArrayBuffer> jvalues = v8::ArrayBuffer::New(isolate, sizeof(glm::quat) * rot.values.size());
			void* p_values = jvalues->GetBackingStore()->Data();
			memcpy(p_values, rot.values.data(), sizeof(glm::quat) * rot.values.size());
			jRot->Set(context, v8::String::NewFromUtf8(isolate, "values").ToLocalChecked(), v8::Float32Array::New(jvalues, 0, rot.values.size() * 4));

			jRotLst->Set(context, (unsigned)j, jRot);
		}
		janim->Set(context, v8::String::NewFromUtf8(isolate, "rotations").ToLocalChecked(), jRotLst);
	}

	size_t num_scales = anim.scales.size();
	if (num_scales > 0)
	{
		v8::Local<v8::Array> jScaleLst = v8::Array::New(isolate, (int)num_scales);
		for (size_t j = 0; j < num_scales; j++)
		{
			const ScaleTrack& scale = anim.scales[j];

			v8::Local<v8::Object> jScale = v8::Object::New(isolate);
			jScale->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::String::NewFromUtf8(isolate, scale.name.c_str()).ToLocalChecked());

			std::string interpolation = interpolation_map[(unsigned)scale.interpolation];
			jScale->Set(context, v8::String::NewFromUtf8(isolate, "interpolation").ToLocalChecked(), v8::String::NewFromUtf8(isolate, interpolation.c_str()).ToLocalChecked());

			v8::Local<v8::ArrayBuffer> jtimes = v8::ArrayBuffer::New(isolate, sizeof(float) * scale.times.size());
			float* p_times = (float*)jtimes->GetBackingStore()->Data();
			memcpy(p_times, scale.times.data(), sizeof(float) * scale.times.size());
			jScale->Set(context, v8::String::NewFromUtf8(isolate, "times").ToLocalChecked(), v8::Float32Array::New(jtimes, 0, scale.times.size()));

			v8::Local<v8::ArrayBuffer> jvalues = v8::ArrayBuffer::New(isolate, sizeof(glm::vec3) * scale.values.size());
			void* p_values = jvalues->GetBackingStore()->Data();
			memcpy(p_values, scale.values.data(), sizeof(glm::vec3) * scale.values.size());
			jScale->Set(context, v8::String::NewFromUtf8(isolate, "values").ToLocalChecked(), v8::Float32Array::New(jvalues, 0, scale.values.size() * 3));

			jScaleLst->Set(context, (unsigned)j, jScale);
		}
		janim->Set(context, v8::String::NewFromUtf8(isolate, "scales").ToLocalChecked(), jScaleLst);
	}

}


inline void janim_to_anim(v8::Isolate* isolate, v8::Local<v8::Object> janim, AnimationClip& anim)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	std::unordered_map<std::string, Interpolation> interpolation_map =
	{
		{"STEP", Interpolation::Step},
		{"LINEAR", Interpolation::Linear},
		{"CUBICSPLINE", Interpolation::CubicSpline}
	};

	v8::Local<v8::Value> jName = janim->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
	v8::String::Utf8Value name(isolate, jName);
	anim.name = *name;
	anim.duration = janim->Get(context, v8::String::NewFromUtf8(isolate, "duration").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();	

	if (janim->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "morphs").ToLocalChecked()).ToChecked())
	{
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
			morph.interpolation = interpolation_map[*uInterpolation];

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

	if (janim->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "translations").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Array> jTransLst = janim->Get(context, v8::String::NewFromUtf8(isolate, "translations").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
		anim.translations.resize(jTransLst->Length());
		for (unsigned i = 0; i < jTransLst->Length(); i++)
		{
			v8::Local<v8::Object> jTrans = jTransLst->Get(context, i).ToLocalChecked().As<v8::Object>();
			TranslationTrack& trans = anim.translations[i];

			v8::Local<v8::Value> jName = jTrans->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
			v8::String::Utf8Value name(isolate, jName);
			trans.name = *name;

			v8::Local<v8::Value> jInterpolation = jTrans->Get(context, v8::String::NewFromUtf8(isolate, "interpolation").ToLocalChecked()).ToLocalChecked();
			v8::String::Utf8Value uInterpolation(isolate, jInterpolation);
			trans.interpolation = interpolation_map[*uInterpolation];
		
			v8::Local<v8::Float32Array> jtimes = jTrans->Get(context, v8::String::NewFromUtf8(isolate, "times").ToLocalChecked()).ToLocalChecked().As<v8::Float32Array>();
			trans.times.resize(jtimes->Length());
			const float* p_times = (const float*)jtimes->Buffer()->GetBackingStore()->Data();
			memcpy(trans.times.data(), p_times, sizeof(float) * jtimes->Length());

			v8::Local<v8::Float32Array> jvalues = jTrans->Get(context, v8::String::NewFromUtf8(isolate, "values").ToLocalChecked()).ToLocalChecked().As<v8::Float32Array>();
			trans.values.resize(jvalues->Length()/3);
			const float* p_values = (const float*)jvalues->Buffer()->GetBackingStore()->Data();
			memcpy(trans.values.data(), p_values, sizeof(float) * jvalues->Length());
		}
	}

	if (janim->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "rotations").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Array> jRotLst = janim->Get(context, v8::String::NewFromUtf8(isolate, "rotations").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
		anim.rotations.resize(jRotLst->Length());
		for (unsigned i = 0; i < jRotLst->Length(); i++)
		{
			v8::Local<v8::Object> jRot = jRotLst->Get(context, i).ToLocalChecked().As<v8::Object>();
			RotationTrack& rot = anim.rotations[i];

			v8::Local<v8::Value> jName = jRot->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
			v8::String::Utf8Value name(isolate, jName);
			rot.name = *name;

			v8::Local<v8::Value> jInterpolation = jRot->Get(context, v8::String::NewFromUtf8(isolate, "interpolation").ToLocalChecked()).ToLocalChecked();
			v8::String::Utf8Value uInterpolation(isolate, jInterpolation);
			rot.interpolation = interpolation_map[*uInterpolation];

			v8::Local<v8::Float32Array> jtimes = jRot->Get(context, v8::String::NewFromUtf8(isolate, "times").ToLocalChecked()).ToLocalChecked().As<v8::Float32Array>();
			rot.times.resize(jtimes->Length());
			const float* p_times = (const float*)jtimes->Buffer()->GetBackingStore()->Data();
			memcpy(rot.times.data(), p_times, sizeof(float) * jtimes->Length());

			v8::Local<v8::Float32Array> jvalues = jRot->Get(context, v8::String::NewFromUtf8(isolate, "values").ToLocalChecked()).ToLocalChecked().As<v8::Float32Array>();
			rot.values.resize(jvalues->Length() / 4);
			const float* p_values = (const float*)jvalues->Buffer()->GetBackingStore()->Data();
			memcpy(rot.values.data(), p_values, sizeof(float) * jvalues->Length());
		}
	}

	if (janim->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "scales").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Array> jScaleLst = janim->Get(context, v8::String::NewFromUtf8(isolate, "scales").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
		anim.scales.resize(jScaleLst->Length());
		for (unsigned i = 0; i < jScaleLst->Length(); i++)
		{
			v8::Local<v8::Object> jScale = jScaleLst->Get(context, i).ToLocalChecked().As<v8::Object>();
			ScaleTrack& scale = anim.scales[i];

			v8::Local<v8::Value> jName = jScale->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
			v8::String::Utf8Value name(isolate, jName);
			scale.name = *name;

			v8::Local<v8::Value> jInterpolation = jScale->Get(context, v8::String::NewFromUtf8(isolate, "interpolation").ToLocalChecked()).ToLocalChecked();
			v8::String::Utf8Value uInterpolation(isolate, jInterpolation);
			scale.interpolation = interpolation_map[*uInterpolation];

			v8::Local<v8::Float32Array> jtimes = jScale->Get(context, v8::String::NewFromUtf8(isolate, "times").ToLocalChecked()).ToLocalChecked().As<v8::Float32Array>();
			scale.times.resize(jtimes->Length());
			const float* p_times = (const float*)jtimes->Buffer()->GetBackingStore()->Data();
			memcpy(scale.times.data(), p_times, sizeof(float) * jtimes->Length());

			v8::Local<v8::Float32Array> jvalues = jScale->Get(context, v8::String::NewFromUtf8(isolate, "values").ToLocalChecked()).ToLocalChecked().As<v8::Float32Array>();
			scale.values.resize(jvalues->Length() / 3);
			const float* p_values = (const float*)jvalues->Buffer()->GetBackingStore()->Data();
			memcpy(scale.values.data(), p_values, sizeof(float) * jvalues->Length());
		}
	}

}