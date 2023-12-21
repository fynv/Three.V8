#include "WrapperUtils.hpp"
#include "core/WrapperObject3D.h"
#include <lights/ProbeGridWidget.h>

#include "WrapperProbeGridWidget.h"

void WrapperProbeGridWidget::define(ClassDefinition& cls)
{
	WrapperObject3D::define(cls);
	cls.name = "ProbeGridWidget";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "coverageMin",  GetCoverageMin },
		{ "coverageMax",  GetCoverageMin },
		{ "divisions",  GetDivisions },
		{ "ypower", GetYpower, SetYpower },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getCoverageMin", GetCoverageMin },
		{"setCoverageMin", SetCoverageMin },
		{"getCoverageMax", GetCoverageMax },
		{"setCoverageMax", SetCoverageMax },
		{"getDivisions", GetDivisions },
		{"setDivisions", SetDivisions },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}



void* WrapperProbeGridWidget::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new ProbeGridWidget;
}

void WrapperProbeGridWidget::GetCoverageMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->coverage_min, position);
	info.GetReturnValue().Set(position);
}

void WrapperProbeGridWidget::GetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
	lctx.vec3_to_jvec3(self->coverage_min, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperProbeGridWidget::SetCoverageMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
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

void WrapperProbeGridWidget::GetCoverageMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->coverage_max, position);
	info.GetReturnValue().Set(position);
}

void WrapperProbeGridWidget::GetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
	lctx.vec3_to_jvec3(self->coverage_max, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperProbeGridWidget::SetCoverageMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
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


void WrapperProbeGridWidget::GetDivisions(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.ivec3_to_jvec3(self->divisions, position);
	info.GetReturnValue().Set(position);
}

void WrapperProbeGridWidget::GetDivisions(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
	lctx.ivec3_to_jvec3(self->divisions, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperProbeGridWidget::SetDivisions(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();

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

}

void WrapperProbeGridWidget::GetYpower(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->ypower));
}

void WrapperProbeGridWidget::SetYpower(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	ProbeGridWidget* self = lctx.self<ProbeGridWidget>();
	lctx.jnum_to_num(value, self->ypower);
}

