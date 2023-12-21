#include "WrapperUtils.hpp"
#include "WrapperIndirectLight.h"
#include <lights/ProbeGrid.h>

#include "WrapperProbeGrid.h"

void WrapperProbeGrid::define(ClassDefinition& cls)
{
	WrapperIndirectLight::define(cls);
	cls.name = "ProbeGrid";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "coverageMin", GetCoverageMin },
		{ "coverageMax", GetCoverageMax },
		{ "divisions", GetDivisions },
		{ "ypower", GetYpower, SetYpower },
		{ "normalBias", GetNormalBias, SetNormalBias },
		{ "perPrimitive", GetPerPrimitive, SetPerPrimitive },
		{ "recordReferences", GetRecordReferences, SetRecordReferences },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getCoverageMin", GetCoverageMin },
		{"setCoverageMin", SetCoverageMin },
		{"getCoverageMax", GetCoverageMax },
		{"setCoverageMax", SetCoverageMax },
		{"getDivisions", GetDivisions },
		{"setDivisions", SetDivisions },
		{"getReferences", GetReferences },
		{"constructVisibility", ConstructVisibility },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());

}

void* WrapperProbeGrid::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new ProbeGrid;
}

void WrapperProbeGrid::GetCoverageMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->coverage_min, position);
	info.GetReturnValue().Set(position);
}

void WrapperProbeGrid::GetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	lctx.vec3_to_jvec3(self->coverage_min, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperProbeGrid::SetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->coverage_min.x);
		lctx.jnum_to_num(info[1], self->coverage_min.y);
		lctx.jnum_to_num(info[2], self->coverage_min.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->coverage_min);
	}
}

void WrapperProbeGrid::GetCoverageMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->coverage_max, position);
	info.GetReturnValue().Set(position);
}

void WrapperProbeGrid::GetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	lctx.vec3_to_jvec3(self->coverage_max, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperProbeGrid::SetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->coverage_max.x);
		lctx.jnum_to_num(info[1], self->coverage_max.y);
		lctx.jnum_to_num(info[2], self->coverage_max.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->coverage_max);
	}
}


void WrapperProbeGrid::GetDivisions(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.ivec3_to_jvec3(self->divisions, position);
	info.GetReturnValue().Set(position);
}

void WrapperProbeGrid::GetDivisions(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	lctx.ivec3_to_jvec3(self->divisions, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperProbeGrid::SetDivisions(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();

	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->divisions.x);
		lctx.jnum_to_num(info[1], self->divisions.y);
		lctx.jnum_to_num(info[2], self->divisions.z);
	}
	else
	{
		lctx.jvec3_to_ivec3(info[0], self->divisions);
	}

	self->allocate_probes();
}


void WrapperProbeGrid::GetYpower(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->ypower));
}

void WrapperProbeGrid::SetYpower(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	lctx.jnum_to_num(value, self->ypower);
}


void WrapperProbeGrid::GetNormalBias(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->normal_bias));
}

void WrapperProbeGrid::SetNormalBias(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	lctx.jnum_to_num(value, self->normal_bias);
}

void WrapperProbeGrid::GetPerPrimitive(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->per_primitive));
}

void WrapperProbeGrid::SetPerPrimitive(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	self->per_primitive = value.As<v8::Boolean>()->Value();
}

void WrapperProbeGrid::GetRecordReferences(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->record_references));
}

void WrapperProbeGrid::SetRecordReferences(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	bool record = value.As<v8::Boolean>()->Value();
	self->set_record_references(record);
}

void WrapperProbeGrid::GetReferences(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();

	std::vector<unsigned> buf;
	self->get_references(buf);

	v8::Local<v8::Array> ret = v8::Array::New(lctx.isolate, (int)buf.size());
	for (int i = 0; i < (int)buf.size(); i++)
	{
		v8::Local<v8::Boolean> referenced = v8::Boolean::New(lctx.isolate, buf[i] > 0);
		ret->Set(lctx.context, i, referenced);
	}
	info.GetReturnValue().Set(ret);
}

void WrapperProbeGrid::ConstructVisibility(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.self<ProbeGrid>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	self->construct_visibility(*scene);
}

