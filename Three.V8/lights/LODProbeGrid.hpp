#pragma once

#include "WrapperUtils.hpp"
#include "IndirectLight.hpp"
#include <lights/LODProbeGrid.h>

class WrapperLODProbeGrid
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

	static void GetBaseDivisions(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetSubDivisionLevel(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetSubDivisionLevel(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetNumberOfProbes(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

	static void Initialize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void ConstructVisibility(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperLODProbeGrid::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperIndirectLight::create_template(isolate, constructor);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "coverageMin").ToLocalChecked(), GetCoverageMin, 0);
	templ->InstanceTemplate()->Set(isolate, "getCoverageMin", v8::FunctionTemplate::New(isolate, GetCoverageMin));
	templ->InstanceTemplate()->Set(isolate, "setCoverageMin", v8::FunctionTemplate::New(isolate, SetCoverageMin));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "coverageMax").ToLocalChecked(), GetCoverageMax, 0);
	templ->InstanceTemplate()->Set(isolate, "getCoverageMax", v8::FunctionTemplate::New(isolate, GetCoverageMax));
	templ->InstanceTemplate()->Set(isolate, "setCoverageMax", v8::FunctionTemplate::New(isolate, SetCoverageMax));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "baseDivisions").ToLocalChecked(), GetBaseDivisions, 0);
	templ->InstanceTemplate()->Set(isolate, "getBaseDivisions", v8::FunctionTemplate::New(isolate, GetBaseDivisions));
	templ->InstanceTemplate()->Set(isolate, "setBaseDivisions", v8::FunctionTemplate::New(isolate, SetBaseDivisions));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "subDivisionLevel").ToLocalChecked(), GetSubDivisionLevel, SetSubDivisionLevel);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "numberOfProbes").ToLocalChecked(), GetNumberOfProbes, 0);

	templ->InstanceTemplate()->Set(isolate, "initialize", v8::FunctionTemplate::New(isolate, Initialize));
	templ->InstanceTemplate()->Set(isolate, "constructVisibility", v8::FunctionTemplate::New(isolate, ConstructVisibility));

	return templ;
}

void WrapperLODProbeGrid::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = new LODProbeGrid();
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), WrapperIndirectLight::dtor);
}

void WrapperLODProbeGrid::GetCoverageMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->coverage_min, position);
	info.GetReturnValue().Set(position);
}

void WrapperLODProbeGrid::GetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	lctx.vec3_to_jvec3(self->coverage_min, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperLODProbeGrid::SetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
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

void WrapperLODProbeGrid::GetCoverageMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->coverage_max, position);
	info.GetReturnValue().Set(position);
}

void WrapperLODProbeGrid::GetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	lctx.vec3_to_jvec3(self->coverage_max, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperLODProbeGrid::SetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
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


void WrapperLODProbeGrid::GetBaseDivisions(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.ivec3_to_jvec3(self->base_divisions, position);
	info.GetReturnValue().Set(position);
}

void WrapperLODProbeGrid::GetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	lctx.ivec3_to_jvec3(self->base_divisions, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperLODProbeGrid::SetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();

	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->base_divisions.x);
		lctx.jnum_to_num(info[1], self->base_divisions.y);
		lctx.jnum_to_num(info[2], self->base_divisions.z);
	}
	else
	{
		lctx.jvec3_to_ivec3(info[0], self->base_divisions);
	}
}


void WrapperLODProbeGrid::Initialize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	GLRenderer *renderer = lctx.jobj_to_obj<GLRenderer>(info[0]);
	Scene* scene = lctx.jobj_to_obj<Scene>(info[1]);
	int budget = -1;
	if (info.Length() > 2)
	{
		lctx.jnum_to_num(info[2], budget);
	}
	self->initialize(*renderer, *scene, budget);

}


void WrapperLODProbeGrid::GetSubDivisionLevel(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->sub_division_level));
}

void WrapperLODProbeGrid::SetSubDivisionLevel(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	lctx.jnum_to_num(value, self->sub_division_level);
}

void WrapperLODProbeGrid::GetNumberOfProbes(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->getNumberOfProbes()));
}


void WrapperLODProbeGrid::ConstructVisibility(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	self->construct_visibility(*scene);
}
