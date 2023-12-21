#include "WrapperUtils.hpp"
#include "lights/WrapperLight.h"
#include <lights/DirectionalLight.h>
#include <lights/DirectionalLightShadow.h>
#include <scenes/Scene.h>

#include "WrapperDirectionalLight.h"

void WrapperDirectionalLight::define(ClassDefinition& cls)
{
	WrapperLight::define(cls);
	cls.name = "DirectionalLight";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "target",  GetTarget, SetTarget },
		{ "bias",  GetBias, SetBias },
		{ "forceCull",   GetForceCull, SetForceCull },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"setShadow", SetShadow },
		{"setShadowProjection", SetShadowProjection },
		{"setShadowRadius", SetShadowRadius },
		{"getBoundingBox", GetBoundingBox },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}


void* WrapperDirectionalLight::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new DirectionalLight;
}

void WrapperDirectionalLight::GetTarget(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> target = v8::Null(lctx.isolate);
	if (lctx.has_property(holder, "_target"))
	{
		target = lctx.get_property(holder, "_target");
	}
	info.GetReturnValue().Set(target);
}

void WrapperDirectionalLight::SetTarget(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();
	if (value->IsNull())
	{
		self->target = nullptr;
	}
	else
	{
		Object3D* target = lctx.jobj_to_obj<Object3D>(value);
		self->target = target;
	}

	v8::Local<v8::Object> holder = info.Holder();
	lctx.set_property(holder, "_target", value);
}

void WrapperDirectionalLight::SetShadow(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();

	bool enable = info[0].As<v8::Boolean>()->Value();
	int width = -1;
	int height = -1;
	if (info.Length() > 2)
	{
		lctx.jnum_to_num(info[1], width);
		lctx.jnum_to_num(info[2], height);
	}
	self->setShadow(enable, width, height);
	self->moved = true;
}

void  WrapperDirectionalLight::SetShadowProjection(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();

	float left, right, bottom, top, zNear, zFar;
	lctx.jnum_to_num(info[0], left);
	lctx.jnum_to_num(info[1], right);
	lctx.jnum_to_num(info[2], bottom);
	lctx.jnum_to_num(info[3], top);
	lctx.jnum_to_num(info[4], zNear);
	lctx.jnum_to_num(info[5], zFar);
	self->setShadowProjection(left, right, bottom, top, zNear, zFar);
	self->moved = true;
}

void  WrapperDirectionalLight::SetShadowRadius(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();

	float radius;
	lctx.jnum_to_num(info[0], radius);
	self->SetShadowRadius(radius);
}


void WrapperDirectionalLight::GetBias(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();
	float bias = 0.001f;
	if (self->shadow != nullptr)
	{
		bias = self->shadow->m_bias;
	}
	info.GetReturnValue().Set(lctx.num_to_jnum(bias));
}

void WrapperDirectionalLight::SetBias(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();
	if (self->shadow != nullptr)
	{
		lctx.jnum_to_num(value, self->shadow->m_bias);
		self->moved = true;
	}
}


void WrapperDirectionalLight::GetForceCull(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();
	bool force_cull = true;
	if (self->shadow != nullptr)
	{
		force_cull = self->shadow->m_force_cull;
	}
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, force_cull));
}

void WrapperDirectionalLight::SetForceCull(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();
	if (self->shadow != nullptr)
	{
		self->shadow->m_force_cull = value.As<v8::Boolean>()->Value();
		self->moved = true;
	}
}


void WrapperDirectionalLight::GetBoundingBox(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	DirectionalLight* self = lctx.self<DirectionalLight>();
	Scene* scene = lctx.jobj_to_obj<Scene>(info[0]);

	self->lookAtTarget();
	self->updateWorldMatrix(false, false);
	glm::mat4 view_matrix = glm::inverse(self->matrixWorld);

	glm::vec3 min_pos, max_pos;
	scene->get_bounding_box(min_pos, max_pos, view_matrix);

	v8::Local<v8::Object> ret = v8::Object::New(lctx.isolate);
	v8::Local<v8::Object> j_min_pos = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(min_pos, j_min_pos);
	v8::Local<v8::Object> j_max_pos = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(max_pos, j_max_pos);
	lctx.set_property(ret, "minPos", j_min_pos);
	lctx.set_property(ret, "maxPos", j_max_pos);
	info.GetReturnValue().Set(ret);
}

