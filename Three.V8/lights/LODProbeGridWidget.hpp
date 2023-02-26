#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <lights/LODProbeGridWidget.h>

class WrapperLODProbeGridWidget
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetCoverageMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetCoverageMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetBaseDivisions(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetBaseDivisions(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetProbeGrid(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetProbeGrid(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};


v8::Local<v8::FunctionTemplate> WrapperLODProbeGridWidget::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "coverageMin").ToLocalChecked(), GetCoverageMin, 0);
	templ->InstanceTemplate()->Set(isolate, "getCoverageMin", v8::FunctionTemplate::New(isolate, GetCoverageMin));
	templ->InstanceTemplate()->Set(isolate, "setCoverageMin", v8::FunctionTemplate::New(isolate, SetCoverageMin));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "coverageMax").ToLocalChecked(), GetCoverageMax, 0);
	templ->InstanceTemplate()->Set(isolate, "getCoverageMax", v8::FunctionTemplate::New(isolate, GetCoverageMax));
	templ->InstanceTemplate()->Set(isolate, "setCoverageMax", v8::FunctionTemplate::New(isolate, SetCoverageMax));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "baseDivisions").ToLocalChecked(), GetBaseDivisions, 0);
	templ->InstanceTemplate()->Set(isolate, "getBaseDivisions", v8::FunctionTemplate::New(isolate, GetBaseDivisions));
	templ->InstanceTemplate()->Set(isolate, "setBaseDivisions", v8::FunctionTemplate::New(isolate, SetBaseDivisions));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "probeGrid").ToLocalChecked(), GetProbeGrid, SetProbeGrid);

	return templ;
}


void WrapperLODProbeGridWidget::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGridWidget* self = new LODProbeGridWidget();
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), WrapperObject3D::dtor);
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

