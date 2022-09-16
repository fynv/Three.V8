#pragma once

#include <string>
#include <vector>
#include <v8.h>
#include <unordered_map>
#include <glm.hpp>
#include <gtx/quaternion.hpp>
#include <models/Animation.h>
#include "GamePlayer.h"

class LocalContext
{
public:	
	v8::Isolate* isolate;
	v8::HandleScope handle_scope;
	v8::Local<v8::Context> context;
	v8::Local<v8::Object> holder;

	template<class InfoType>
	LocalContext(const InfoType& info)
		: isolate(info.GetIsolate())
		, handle_scope(isolate)
		, context(isolate->GetCurrentContext())
		, holder(info.Holder())
	{
	
	}

	LocalContext(v8::Isolate* isolate)
		: isolate(isolate)
		, handle_scope(isolate)
		, context(isolate->GetCurrentContext())
	{

	}

	inline GameContext* ctx()
	{
		v8::Local<v8::Object> global = context->Global();
		GameContext* ctx = (GameContext*)global->GetAlignedPointerFromInternalField(0);
		return ctx;
	}

	inline GamePlayer* player()
	{
		GameContext* _ctx = ctx();
		return _ctx->GetGamePlayer();
	}

	inline void* get_self()
	{
		return holder->GetAlignedPointerFromInternalField(0);
	}

	template<class T>
	inline T* self()
	{
		return (T*)get_self();
	}

	template<class T>
	inline T* jobj_to_obj(v8::Local<v8::Value> obj)
	{
		return (T*)obj.As<v8::Object>()->GetAlignedPointerFromInternalField(0);
	}

	v8::Local<v8::Object> instantiate(const char* cls_name)
	{
		v8::Local<v8::Object> global = context->Global();
		v8::Local<v8::Function> ctor = get_property(global, cls_name).As<v8::Function>();
		return ctor->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	}

	inline bool has_property(v8::Local<v8::Object> obj, const char* name)
	{
		return obj->Has(context, v8::String::NewFromUtf8(isolate, name).ToLocalChecked()).ToChecked();
	}

	inline v8::Local<v8::Value> get_property(v8::Local<v8::Object> obj, const char* name)
	{
		v8::Local<v8::Value> prop = v8::Null(isolate);
		if (has_property(obj, name))
		{
			prop = obj->Get(context, v8::String::NewFromUtf8(isolate, name).ToLocalChecked()).ToLocalChecked();
		}
		return prop;
	}

	inline void set_property(v8::Local<v8::Object> obj, const char* name, v8::Local<v8::Value> value)
	{
		obj->Set(context, v8::String::NewFromUtf8(isolate, name).ToLocalChecked(), value);
	}

	inline void del_property(v8::Local<v8::Object> obj, const char* name)
	{
		obj->Delete(context, v8::String::NewFromUtf8(isolate, name).ToLocalChecked());
	}

	template<typename T>
	inline v8::Local<v8::Number> num_to_jnum(T value)
	{
		return v8::Number::New(isolate, (double)value);
	}

	template<typename T>
	inline void jnum_to_num(v8::Local<v8::Value> jnum, T& value)
	{
		value = (T)jnum.As<v8::Number>()->Value();
	}

	inline v8::Local<v8::String> str_to_jstr(const char* str)
	{
		return v8::String::NewFromUtf8(isolate, str).ToLocalChecked();
	}

	inline std::string jstr_to_str(v8::Local<v8::Value> value)
	{
		v8::String::Utf8Value str(isolate, value);
		return *str;
	}


	inline void vec2_to_jvec2(const glm::vec2& vec, v8::Local<v8::Value> jvec)
	{
		v8::Local<v8::Object> obj = jvec.As<v8::Object>();
		set_property(obj, "x", num_to_jnum(vec.x));
		set_property(obj, "y", num_to_jnum(vec.y));		
	}

	inline void jvec2_to_vec2(v8::Local<v8::Value> jvec, glm::vec2& vec)
	{
		v8::Local<v8::Object> obj = jvec.As<v8::Object>();
		jnum_to_num(get_property(obj, "x"), vec.x);
		jnum_to_num(get_property(obj, "y"), vec.y);
	}

	inline void vec3_to_jvec3(const glm::vec3& vec, v8::Local<v8::Value> jvec)
	{
		v8::Local<v8::Object> obj = jvec.As<v8::Object>();
		set_property(obj, "x", num_to_jnum(vec.x));
		set_property(obj, "y", num_to_jnum(vec.y));
		set_property(obj, "z", num_to_jnum(vec.z));
	}


	inline void jvec3_to_vec3(v8::Local<v8::Value> jvec, glm::vec3& vec)
	{
		v8::Local<v8::Object> obj = jvec.As<v8::Object>();
		jnum_to_num(get_property(obj, "x"), vec.x);
		jnum_to_num(get_property(obj, "y"), vec.y);
		jnum_to_num(get_property(obj, "z"), vec.z);
	}

	inline void quat_to_jquat(const glm::quat& quat, v8::Local<v8::Value> jquat)
	{
		v8::Local<v8::Object> obj = jquat.As<v8::Object>();
		set_property(obj, "x", num_to_jnum(quat.x));
		set_property(obj, "y", num_to_jnum(quat.y));
		set_property(obj, "z", num_to_jnum(quat.z));
		set_property(obj, "w", num_to_jnum(quat.w));
	}

	inline void jquat_to_quat(v8::Local<v8::Value> jquat, glm::quat& quat)
	{
		v8::Local<v8::Object> obj = jquat.As<v8::Object>();
		jnum_to_num(get_property(obj, "x"), quat.x);
		jnum_to_num(get_property(obj, "y"), quat.y);
		jnum_to_num(get_property(obj, "z"), quat.z);
		jnum_to_num(get_property(obj, "w"), quat.w);
	}


	inline void mat4_to_jmat4(const glm::mat4& matrix, v8::Local<v8::Value> jmatrix)
	{
		v8::Local<v8::Object> obj = jmatrix.As<v8::Object>();

		v8::Local<v8::Array> elements;
		if (has_property(obj, "elements"))
		{
			elements = get_property(obj, "elements").As<v8::Array>();
		}
		else
		{
			elements = v8::Array::New(isolate, 16);
			set_property(obj, "elements", elements);
		}
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				elements->Set(context, j + i * 4, num_to_jnum(matrix[i][j]));
			}
		}
	}

	inline void jmat4_to_mat4(v8::Local<v8::Value> jmatrix, glm::mat4& matrix)
	{
		v8::Local<v8::Object> obj = jmatrix.As<v8::Object>();

		v8::Local<v8::Array> elements = get_property(obj, "elements").As<v8::Array>();
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				jnum_to_num(elements->Get(context, j + i * 4).ToLocalChecked(), matrix[i][j]);
			}
		}
	}


	inline void jframe_to_frame(v8::Local<v8::Value> jFrame, AnimationFrame& frame)
	{
		v8::Local<v8::Object> obj = jFrame.As<v8::Object>();

		if (has_property(obj, "morphs"))
		{
			v8::Local<v8::Array> jMorphs = get_property(obj, "morphs").As<v8::Array>();
			frame.morphs.resize(jMorphs->Length());
			for (unsigned i = 0; i < jMorphs->Length(); i++)
			{
				v8::Local<v8::Object> jMorph = jMorphs->Get(context, i).ToLocalChecked().As<v8::Object>();
				v8::Local<v8::Value> jName = get_property(jMorph, "name");
				std::string name = jstr_to_str(jName);
				v8::Local<v8::Array> jWeights = get_property(jMorph, "weights").As<v8::Array>();

				MorphFrame& morph = frame.morphs[i];
				morph.name = name;
				morph.weights.resize(jWeights->Length());
				for (unsigned j = 0; j < jWeights->Length(); j++)
				{
					jnum_to_num(jWeights->Get(context, j).ToLocalChecked(), morph.weights[j]);
				}
			}
		}

		if (has_property(obj, "translations"))
		{
			v8::Local<v8::Array> jTransLst = get_property(obj, "translations").As<v8::Array>();
			frame.translations.resize(jTransLst->Length());
			for (unsigned i = 0; i < jTransLst->Length(); i++)
			{
				v8::Local<v8::Object> jTransFrame = jTransLst->Get(context, i).ToLocalChecked().As<v8::Object>();
				v8::Local<v8::Value> jName = get_property(jTransFrame, "name");
				std::string name = jstr_to_str(jName);

				TranslationFrame& trans = frame.translations[i];
				trans.name = name;
				v8::Local<v8::Value> jTranslation = get_property(jTransFrame, "translation");
				jvec3_to_vec3(jTranslation, trans.translation);
			}
		}

		if (has_property(obj, "rotations"))
		{			
			v8::Local<v8::Array> jRotLst = get_property(obj, "rotations").As<v8::Array>();
			frame.rotations.resize(jRotLst->Length());
			for (unsigned i = 0; i < jRotLst->Length(); i++)
			{
				v8::Local<v8::Object> jRotFrame = jRotLst->Get(context, i).ToLocalChecked().As<v8::Object>();
				v8::Local<v8::Value> jName = get_property(jRotFrame, "name");
				std::string name = jstr_to_str(jName);			

				RotationFrame& rot = frame.rotations[i];
				rot.name = name;
				v8::Local<v8::Value> jRotation = get_property(jRotFrame, "rotation");
				jquat_to_quat(jRotation, rot.rotation);				
			}
		}

		if (has_property(obj, "scales"))
		{
			v8::Local<v8::Array> jScaleLst = get_property(obj, "scales").As<v8::Array>();
			frame.rotations.resize(jScaleLst->Length());
			for (unsigned i = 0; i < jScaleLst->Length(); i++)
			{
				v8::Local<v8::Object> jScaleFrame = jScaleLst->Get(context, i).ToLocalChecked().As<v8::Object>();
				v8::Local<v8::Value> jName = get_property(jScaleFrame, "name");
				std::string name = jstr_to_str(jName);

				ScaleFrame& scale = frame.scales[i];
				scale.name = name;
				v8::Local<v8::Value> jScale = get_property(jScaleFrame, "scale");
				jvec3_to_vec3(jScale, scale.scale);
			}

		}		
	}

	inline void anim_to_janim(const AnimationClip& anim, v8::Local<v8::Value> janim)
	{
		v8::Local<v8::Object> obj = janim.As<v8::Object>();
		
		static std::string interpolation_map[3] = { "STEP", "LINEAR", "CUBICSPLINE" };

		set_property(obj, "name", str_to_jstr(anim.name.c_str()));
		set_property(obj, "duration", num_to_jnum(anim.duration));

		size_t num_morphs = anim.morphs.size();
		if (num_morphs > 0)
		{
			v8::Local<v8::Array> jmorphs = v8::Array::New(isolate, (int)num_morphs);
			for (size_t j = 0; j < num_morphs; j++)
			{
				const MorphTrack& morph = anim.morphs[j];

				v8::Local<v8::Object> jmorph = v8::Object::New(isolate);
				set_property(jmorph, "name", str_to_jstr(morph.name.c_str()));
				set_property(jmorph, "targets", num_to_jnum(morph.num_targets));

				std::string interpolation = interpolation_map[(unsigned)morph.interpolation];
				set_property(jmorph, "interpolation", str_to_jstr(interpolation.c_str()));

				v8::Local<v8::ArrayBuffer> jtimes = v8::ArrayBuffer::New(isolate, sizeof(float) * morph.times.size());
				float* p_times = (float*)jtimes->GetBackingStore()->Data();
				memcpy(p_times, morph.times.data(), sizeof(float) * morph.times.size());
				set_property(jmorph, "times", v8::Float32Array::New(jtimes, 0, morph.times.size()));

				v8::Local<v8::ArrayBuffer> jvalues = v8::ArrayBuffer::New(isolate, sizeof(float) * morph.values.size());
				void* p_values = jvalues->GetBackingStore()->Data();
				memcpy(p_values, morph.values.data(), sizeof(float) * morph.values.size());
				set_property(jmorph, "values", v8::Float32Array::New(jvalues, 0, morph.values.size()));

				jmorphs->Set(context, (unsigned)j, jmorph);
			}
			set_property(obj, "morphs", jmorphs);
		}

		size_t num_trans = anim.translations.size();
		if (num_trans > 0)
		{
			v8::Local<v8::Array> jTransLst = v8::Array::New(isolate, (int)num_trans);
			for (size_t j = 0; j < num_trans; j++)
			{
				const TranslationTrack& trans = anim.translations[j];

				v8::Local<v8::Object> jTrans = v8::Object::New(isolate);
				set_property(jTrans, "name", str_to_jstr(trans.name.c_str()));

				std::string interpolation = interpolation_map[(unsigned)trans.interpolation];
				set_property(jTrans, "interpolation", str_to_jstr(interpolation.c_str()));

				v8::Local<v8::ArrayBuffer> jtimes = v8::ArrayBuffer::New(isolate, sizeof(float) * trans.times.size());
				float* p_times = (float*)jtimes->GetBackingStore()->Data();
				memcpy(p_times, trans.times.data(), sizeof(float) * trans.times.size());
				set_property(jTrans, "times", v8::Float32Array::New(jtimes, 0, trans.times.size()));

				v8::Local<v8::ArrayBuffer> jvalues = v8::ArrayBuffer::New(isolate, sizeof(glm::vec3) * trans.values.size());
				void* p_values = jvalues->GetBackingStore()->Data();
				memcpy(p_values, trans.values.data(), sizeof(glm::vec3) * trans.values.size());
				set_property(jTrans, "values", v8::Float32Array::New(jvalues, 0, trans.values.size() * 3));

				jTransLst->Set(context, (unsigned)j, jTrans);
			}
			set_property(obj, "translations", jTransLst);
		}

		size_t num_rots = anim.rotations.size();
		if (num_rots > 0)
		{
			v8::Local<v8::Array> jRotLst = v8::Array::New(isolate, (int)num_rots);
			for (size_t j = 0; j < num_rots; j++)
			{
				const RotationTrack& rot = anim.rotations[j];

				v8::Local<v8::Object> jRot = v8::Object::New(isolate);
				set_property(jRot, "name", str_to_jstr(rot.name.c_str()));

				std::string interpolation = interpolation_map[(unsigned)rot.interpolation];
				set_property(jRot, "interpolation", str_to_jstr(interpolation.c_str()));

				v8::Local<v8::ArrayBuffer> jtimes = v8::ArrayBuffer::New(isolate, sizeof(float) * rot.times.size());
				float* p_times = (float*)jtimes->GetBackingStore()->Data();
				memcpy(p_times, rot.times.data(), sizeof(float) * rot.times.size());
				set_property(jRot, "times", v8::Float32Array::New(jtimes, 0, rot.times.size()));

				v8::Local<v8::ArrayBuffer> jvalues = v8::ArrayBuffer::New(isolate, sizeof(glm::quat) * rot.values.size());
				void* p_values = jvalues->GetBackingStore()->Data();
				memcpy(p_values, rot.values.data(), sizeof(glm::quat) * rot.values.size());
				set_property(jRot, "values", v8::Float32Array::New(jvalues, 0, rot.values.size() * 4));

				jRotLst->Set(context, (unsigned)j, jRot);
			}
			set_property(obj, "rotations", jRotLst);
		}

		size_t num_scales = anim.scales.size();
		if (num_scales > 0)
		{
			v8::Local<v8::Array> jScaleLst = v8::Array::New(isolate, (int)num_scales);
			for (size_t j = 0; j < num_scales; j++)
			{
				const ScaleTrack& scale = anim.scales[j];

				v8::Local<v8::Object> jScale = v8::Object::New(isolate);
				set_property(jScale, "name", str_to_jstr(scale.name.c_str()));				

				std::string interpolation = interpolation_map[(unsigned)scale.interpolation];
				set_property(jScale, "interpolation", str_to_jstr(interpolation.c_str()));

				v8::Local<v8::ArrayBuffer> jtimes = v8::ArrayBuffer::New(isolate, sizeof(float) * scale.times.size());
				float* p_times = (float*)jtimes->GetBackingStore()->Data();
				memcpy(p_times, scale.times.data(), sizeof(float) * scale.times.size());
				set_property(jScale, "times", v8::Float32Array::New(jtimes, 0, scale.times.size()));

				v8::Local<v8::ArrayBuffer> jvalues = v8::ArrayBuffer::New(isolate, sizeof(glm::vec3) * scale.values.size());
				void* p_values = jvalues->GetBackingStore()->Data();
				memcpy(p_values, scale.values.data(), sizeof(glm::vec3) * scale.values.size());
				set_property(jScale, "values", v8::Float32Array::New(jvalues, 0, scale.values.size() * 3));

				jScaleLst->Set(context, (unsigned)j, jScale);
			}
			set_property(obj, "scales", jScaleLst);
		}

	}


	inline void janim_to_anim(v8::Local<v8::Value> janim, AnimationClip& anim)
	{
		v8::Local<v8::Object> obj = janim.As<v8::Object>();

		std::unordered_map<std::string, Interpolation> interpolation_map =
		{
			{"STEP", Interpolation::Step},
			{"LINEAR", Interpolation::Linear},
			{"CUBICSPLINE", Interpolation::CubicSpline}
		};

		v8::Local<v8::Value> jName = get_property(obj, "name");
		std::string name = jstr_to_str(jName);
		anim.name = name;
		jnum_to_num(get_property(obj, "duration"), anim.duration);

		if (has_property(obj, "morphs"))
		{
			v8::Local<v8::Array> jMorphs = get_property(obj, "morphs").As<v8::Array>();
			anim.morphs.resize(jMorphs->Length());
			for (unsigned i = 0; i < jMorphs->Length(); i++)
			{
				v8::Local<v8::Object> jMorph = jMorphs->Get(context, i).ToLocalChecked().As<v8::Object>();
				MorphTrack& morph = anim.morphs[i];
				v8::Local<v8::Value> jName = get_property(jMorph, "name");
				std::string name = jstr_to_str(jName);
				morph.name = name;
				jnum_to_num(get_property(jMorph, "targets"), morph.num_targets);

				v8::Local<v8::Value> jInterpolation = get_property(jMorph, "interpolation");
				std::string interpolation = jstr_to_str(jInterpolation);
				morph.interpolation = interpolation_map[interpolation];

				v8::Local<v8::Float32Array> jtimes = get_property(jMorph,"times").As<v8::Float32Array>();
				morph.times.resize(jtimes->Length());
				const float* p_times = (const float*)jtimes->Buffer()->GetBackingStore()->Data();
				memcpy(morph.times.data(), p_times, sizeof(float) * jtimes->Length());

				v8::Local<v8::Float32Array> jvalues = get_property(jMorph, "values").As<v8::Float32Array>();
				morph.values.resize(jvalues->Length());
				const float* p_values = (const float*)jvalues->Buffer()->GetBackingStore()->Data();
				memcpy(morph.values.data(), p_values, sizeof(float) * jvalues->Length());

			}
		}

		if (has_property(obj, "translations"))
		{
			v8::Local<v8::Array> jTransLst = get_property(obj, "translations").As<v8::Array>();
			anim.translations.resize(jTransLst->Length());
			for (unsigned i = 0; i < jTransLst->Length(); i++)
			{
				v8::Local<v8::Object> jTrans = jTransLst->Get(context, i).ToLocalChecked().As<v8::Object>();
				TranslationTrack& trans = anim.translations[i];

				v8::Local<v8::Value> jName = get_property(jTrans, "name");
				std::string name = jstr_to_str(jName);
				trans.name = name;

				v8::Local<v8::Value> jInterpolation = get_property(jTrans, "interpolation");
				std::string interpolation = jstr_to_str(jInterpolation);
				trans.interpolation = interpolation_map[interpolation];

				v8::Local<v8::Float32Array> jtimes = get_property(jTrans, "times").As<v8::Float32Array>();
				trans.times.resize(jtimes->Length());
				const float* p_times = (const float*)jtimes->Buffer()->GetBackingStore()->Data();
				memcpy(trans.times.data(), p_times, sizeof(float) * jtimes->Length());

				v8::Local<v8::Float32Array> jvalues = get_property(jTrans, "values").As<v8::Float32Array>();					
				trans.values.resize(jvalues->Length() / 3);
				const float* p_values = (const float*)jvalues->Buffer()->GetBackingStore()->Data();
				memcpy(trans.values.data(), p_values, sizeof(float) * jvalues->Length());

			}
		}

		if (has_property(obj, "rotations"))
		{
			v8::Local<v8::Array> jRotLst = get_property(obj, "rotations").As<v8::Array>();
			anim.rotations.resize(jRotLst->Length());
			for (unsigned i = 0; i < jRotLst->Length(); i++)
			{
				v8::Local<v8::Object> jRot = jRotLst->Get(context, i).ToLocalChecked().As<v8::Object>();
				RotationTrack& rot = anim.rotations[i];

				v8::Local<v8::Value> jName = get_property(jRot, "name");
				std::string name = jstr_to_str(jName);
				rot.name = name;

				v8::Local<v8::Value> jInterpolation = get_property(jRot, "interpolation");
				std::string interpolation = jstr_to_str(jInterpolation);
				rot.interpolation = interpolation_map[interpolation];

				v8::Local<v8::Float32Array> jtimes = get_property(jRot, "times").As<v8::Float32Array>();
				rot.times.resize(jtimes->Length());
				const float* p_times = (const float*)jtimes->Buffer()->GetBackingStore()->Data();
				memcpy(rot.times.data(), p_times, sizeof(float) * jtimes->Length());

				v8::Local<v8::Float32Array> jvalues = get_property(jRot, "values").As<v8::Float32Array>(); 				
				rot.values.resize(jvalues->Length() / 4);
				const float* p_values = (const float*)jvalues->Buffer()->GetBackingStore()->Data();
				memcpy(rot.values.data(), p_values, sizeof(float) * jvalues->Length());
			}
		}

		if (has_property(obj, "scales"))
		{
			v8::Local<v8::Array> jScaleLst = get_property(obj, "scales").As<v8::Array>();
			anim.scales.resize(jScaleLst->Length());
			for (unsigned i = 0; i < jScaleLst->Length(); i++)
			{
				v8::Local<v8::Object> jScale = jScaleLst->Get(context, i).ToLocalChecked().As<v8::Object>();
				ScaleTrack& scale = anim.scales[i];

				v8::Local<v8::Value> jName = get_property(jScale, "name");
				std::string name = jstr_to_str(jName);
				scale.name = name;

				v8::Local<v8::Value> jInterpolation = get_property(jScale, "interpolation");
				std::string interpolation = jstr_to_str(jInterpolation);
				scale.interpolation = interpolation_map[interpolation];

				v8::Local<v8::Float32Array> jtimes = get_property(jScale, "times").As<v8::Float32Array>();
				scale.times.resize(jtimes->Length());
				const float* p_times = (const float*)jtimes->Buffer()->GetBackingStore()->Data();
				memcpy(scale.times.data(), p_times, sizeof(float)* jtimes->Length());

				v8::Local<v8::Float32Array> jvalues = get_property(jScale, "values").As<v8::Float32Array>();
				scale.values.resize(jvalues->Length() / 3);
				const float* p_values = (const float*)jvalues->Buffer()->GetBackingStore()->Data();
				memcpy(scale.values.data(), p_values, sizeof(float)* jvalues->Length());
			}

		}

	}
};

void GeneralDispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GameContext* ctx = lctx.ctx();
	void* self = lctx.get_self();
	ctx->remove_object(self);
}

inline std::vector<std::string> SplitWithCharacters(const std::string& str, int splitLength) 
{
	int NumSubstrings = str.length() / splitLength;
	std::vector<std::string> ret;

	for (int i = 0; i < NumSubstrings; i++) {
		ret.push_back(str.substr(i * splitLength, splitLength));
	}
	
	if (str.length() % splitLength != 0) 
	{
		ret.push_back(str.substr(splitLength * NumSubstrings));
	}

	return ret;
}

inline void string_to_color(const char* str, glm::u8vec4& color)
{
	std::string hex = str;
	if (hex.at(0) == '#') {
		hex.erase(0, 1);
	}

	while (hex.length() != 6) {
		hex += "0";
	}

	std::vector<std::string> colori = SplitWithCharacters(hex, 2);

	color.a = (uint8_t)stoi(colori[0], nullptr, 16);
	color.r = (uint8_t)stoi(colori[1], nullptr, 16);
	color.g = (uint8_t)stoi(colori[2], nullptr, 16);
	color.b = (uint8_t)stoi(colori[3], nullptr, 16);

}