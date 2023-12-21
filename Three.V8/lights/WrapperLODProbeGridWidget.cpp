#include "WrapperUtils.hpp"
#include "core/WrapperObject3D.h"
#include <lights/LODProbeGridWidget.h>

#include "WrapperLODProbeGridWidget.h"

void WrapperLODProbeGridWidget::define(ClassDefinition& cls)
{
	WrapperObject3D::define(cls);
	cls.name = "LODProbeGridWidget";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "coverageMin",  GetCoverageMin },
		{ "coverageMax",  GetCoverageMin },
		{ "baseDivisions",  GetBaseDivisions },
		{ "probeGrid", GetProbeGrid, SetProbeGrid },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getCoverageMin", GetCoverageMin },
		{"setCoverageMin", SetCoverageMin },
		{"getCoverageMax", GetCoverageMax },
		{"setCoverageMax", SetCoverageMax },
		{"getBaseDivisions", GetBaseDivisions },
		{"setBaseDivisions", SetBaseDivisions },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}

void* WrapperLODProbeGridWidget::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new LODProbeGridWidget;
}

void WrapperLODProbeGridWidget::GetCoverageMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->coverage_min, position);
	info.GetReturnValue().Set(position);
}

void WrapperLODProbeGridWidget::GetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();
	lctx.vec3_to_jvec3(self->coverage_min, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperLODProbeGridWidget::SetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();
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

void WrapperLODProbeGridWidget::GetCoverageMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->coverage_max, position);
	info.GetReturnValue().Set(position);
}

void WrapperLODProbeGridWidget::GetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();
	lctx.vec3_to_jvec3(self->coverage_max, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperLODProbeGridWidget::SetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();
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


void WrapperLODProbeGridWidget::GetBaseDivisions(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.ivec3_to_jvec3(self->base_divisions, position);
	info.GetReturnValue().Set(position);
}

void WrapperLODProbeGridWidget::GetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();
	lctx.ivec3_to_jvec3(self->base_divisions, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperLODProbeGridWidget::SetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();

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

void WrapperLODProbeGridWidget::GetProbeGrid(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> parent = lctx.get_property(info.Holder(), "_probeGrid");
	info.GetReturnValue().Set(parent);
}

void WrapperLODProbeGridWidget::SetProbeGrid(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(info.Holder(), "_probeGrid", value);

	LODProbeGridWidget* self = lctx.self<LODProbeGridWidget>();
	LODProbeGrid* grid = lctx.jobj_to_obj<LODProbeGrid>(value);
	self->probe_grid = grid;
}

