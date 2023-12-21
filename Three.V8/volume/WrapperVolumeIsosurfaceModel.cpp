#include "WrapperUtils.hpp"
#include "core/WrapperObject3D.h"
#include "volume/VolumeData.h"
#include "volume/VolumeIsosurfaceModel.h"
#include "WrapperVolumeIsosurfaceModel.h"


void WrapperVolumeIsosurfaceModel::define(ClassDefinition& cls)
{
	WrapperObject3D::define(cls);
	cls.name = "VolumeIsosurfaceModel";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "isovalue",  GetIsovalue, SetIsovalue },
		{ "color",  GetColor },
		{ "metalness", GetMetalness, SetMetalness },
		{ "roughness",  GetRoughness, SetRoughness },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {		
		{"getColor", GetColor },
		{"setColor", SetColor },		
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}

void* WrapperVolumeIsosurfaceModel::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	lctx.set_property(info.Holder(), "_data", info[0]);

	VolumeData* data = lctx.jobj_to_obj<VolumeData>(info[0]);
	return new VolumeIsosurfaceModel(data);
}

void WrapperVolumeIsosurfaceModel::GetIsovalue(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->m_isovalue));
}

void WrapperVolumeIsosurfaceModel::SetIsovalue(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	lctx.jnum_to_num<float>(value, self->m_isovalue);
}

void WrapperVolumeIsosurfaceModel::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	v8::Local<v8::Object> color = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->m_material.color, color);
	info.GetReturnValue().Set(color);
}

void WrapperVolumeIsosurfaceModel::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	lctx.vec3_to_jvec3(self->m_material.color, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperVolumeIsosurfaceModel::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	glm::vec3 color;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], color.x);
		lctx.jnum_to_num(info[1], color.y);
		lctx.jnum_to_num(info[2], color.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], color);
	}
	self->set_color(color);
}

void WrapperVolumeIsosurfaceModel::GetMetalness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->m_material.metallicFactor));
}

void WrapperVolumeIsosurfaceModel::SetMetalness(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	float metalness;
	lctx.jnum_to_num(value, metalness);
	self->set_metalness(metalness);
}


void WrapperVolumeIsosurfaceModel::GetRoughness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->m_material.roughnessFactor));
}

void WrapperVolumeIsosurfaceModel::SetRoughness(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	VolumeIsosurfaceModel* self = lctx.self<VolumeIsosurfaceModel>();
	float roughness;
	lctx.jnum_to_num(value, roughness);
	self->set_roughness(roughness);
}

