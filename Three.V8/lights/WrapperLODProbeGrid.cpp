#include "WrapperUtils.hpp"
#include "WrapperIndirectLight.h"
#include <lights/LODProbeGrid.h>

#include "WrapperLODProbeGrid.h"

void WrapperLODProbeGrid::define(ClassDefinition& cls)
{
	WrapperIndirectLight::define(cls);
	cls.name = "LODProbeGrid";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "coverageMin", GetCoverageMin },
		{ "coverageMax", GetCoverageMax },
		{ "baseDivisions", GetBaseDivisions },
		{ "subDivisionLevel", GetSubDivisionLevel, SetSubDivisionLevel },
		{ "numberOfProbes", GetNumberOfProbes },
		{ "normalBias", GetNormalBias, SetNormalBias },
		{ "perPrimitive", GetPerPrimitive, SetPerPrimitive },		
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getCoverageMin", GetCoverageMin },
		{"setCoverageMin", SetCoverageMin },
		{"getCoverageMax", GetCoverageMax },
		{"setCoverageMax", SetCoverageMax },
		{"getBaseDivisions", GetBaseDivisions },
		{"setBaseDivisions", SetBaseDivisions },
		{"initialize", Initialize },
		{"constructVisibility", ConstructVisibility },
		{"toProbeGrid", ToProbeGrid },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());

}

void* WrapperLODProbeGrid::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new LODProbeGrid;
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
	GLRenderer* renderer = lctx.jobj_to_obj<GLRenderer>(info[0]);
	Scene* scene = lctx.jobj_to_obj<Scene>(info[1]);
	self->initialize(*renderer, *scene);

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


void WrapperLODProbeGrid::GetNormalBias(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->normal_bias));
}

void WrapperLODProbeGrid::SetNormalBias(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	lctx.jnum_to_num(value, self->normal_bias);
}


void WrapperLODProbeGrid::GetPerPrimitive(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, self->per_primitive));
}

void WrapperLODProbeGrid::SetPerPrimitive(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	self->per_primitive = value.As<v8::Boolean>()->Value();
}

void WrapperLODProbeGrid::ConstructVisibility(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	self->construct_visibility(*scene);
}

void WrapperLODProbeGrid::ToProbeGrid(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.self<LODProbeGrid>();

	v8::Local<v8::Object> holder_out = lctx.instantiate("ProbeGrid");
	ProbeGrid* grid_out = lctx.jobj_to_obj<ProbeGrid>(holder_out);
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);
	self->ToProbeGrid(grid_out, *scene);
	info.GetReturnValue().Set(holder_out);
}

