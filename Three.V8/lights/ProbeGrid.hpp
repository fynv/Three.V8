#pragma once

#include "WrapperUtils.hpp"
#include "IndirectLight.hpp"
#include <lights/ProbeGrid.h>

class WrapperProbeGrid
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetCoverageMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetCoverageMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetDivisions(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetDivisions(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetDivisions(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperProbeGrid::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperIndirectLight::create_template(isolate, constructor);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "coverageMin").ToLocalChecked(), GetCoverageMin, 0);
	templ->InstanceTemplate()->Set(isolate, "getCoverageMin", v8::FunctionTemplate::New(isolate, GetCoverageMin));
	templ->InstanceTemplate()->Set(isolate, "setCoverageMin", v8::FunctionTemplate::New(isolate, SetCoverageMin));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "coverageMax").ToLocalChecked(), GetCoverageMax, 0);
	templ->InstanceTemplate()->Set(isolate, "getCoverageMax", v8::FunctionTemplate::New(isolate, GetCoverageMax));
	templ->InstanceTemplate()->Set(isolate, "setCoverageMax", v8::FunctionTemplate::New(isolate, SetCoverageMax));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "divisions").ToLocalChecked(), GetDivisions, 0);
	templ->InstanceTemplate()->Set(isolate, "getDivisions", v8::FunctionTemplate::New(isolate, GetDivisions));
	templ->InstanceTemplate()->Set(isolate, "setDivisions", v8::FunctionTemplate::New(isolate, SetDivisions));

	return templ;
}

void WrapperProbeGrid::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = new ProbeGrid();
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), WrapperIndirectLight::dtor);
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