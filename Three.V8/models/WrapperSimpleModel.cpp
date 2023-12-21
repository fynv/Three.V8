#pragma once

#include "WrapperUtils.hpp"
#include "core/WrapperObject3D.h"
#include <renderers/GLRenderTarget.h>
#if THREE_MM
#include <MMCamera.h>
#include <MMLazyVideo.h>
#include <MMPlayer.h>
#endif
#include <models/SimpleModel.h>
#include <models/GeometryCreator.h>
#include <utils/Image.h>

#include "WrapperSimpleModel.h"

void WrapperSimpleModel::define(ClassDefinition& cls)
{
	WrapperObject3D::define(cls);
	cls.name = "SimpleModel";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "color",  GetColor },
		{ "metalness", GetMetalness, SetMetalness },
		{ "roughness",  GetRoughness, SetRoughness },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"createBox", CreateBox },
		{"createSphere", CreateSphere },
		{"createPlane", CreatePlane },
		{"getColor", GetColor },
		{"setColor", SetColor },
		{"setColorTexture", SetColorTexture },
		{"setToonShading", SetToonShading },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}

void* WrapperSimpleModel::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new SimpleModel;
}

void WrapperSimpleModel::CreateBox(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	float width, height, depth;
	lctx.jnum_to_num(info[0], width);
	lctx.jnum_to_num(info[1], height);
	lctx.jnum_to_num(info[2], depth);
	GeometryCreator::CreateBox(&self->geometry, width, height, depth);
}

void WrapperSimpleModel::CreateSphere(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	float radius;
	lctx.jnum_to_num(info[0], radius);
	int widthSegments = 32;
	int heightSegments = 16;
	if (info.Length() > 1)
	{
		lctx.jnum_to_num(info[1], widthSegments);
		if (info.Length() > 2)
		{
			lctx.jnum_to_num(info[2], heightSegments);
		}
	}
	GeometryCreator::CreateSphere(&self->geometry, radius, widthSegments, heightSegments);
}


void WrapperSimpleModel::CreatePlane(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	float width, height;
	lctx.jnum_to_num(info[0], width);
	lctx.jnum_to_num(info[1], height);
	GeometryCreator::CreatePlane(&self->geometry, width, height);
}

void WrapperSimpleModel::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	v8::Local<v8::Object> color = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->material.color, color);
	info.GetReturnValue().Set(color);
}

void WrapperSimpleModel::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	lctx.vec3_to_jvec3(self->material.color, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperSimpleModel::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
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

void WrapperSimpleModel::SetColorTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();

	if (info[0]->IsNull())
	{
		glm::u8vec3 white = { 255, 255, 255 };
		self->texture.load_memory_bgr(1, 1, (uint8_t*)&white, true);
		self->repl_texture = nullptr;
		return;
	}

	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();

	std::string clsname = lctx.jstr_to_str(holder_image->GetConstructorName());
	if (clsname == "Image")
	{
		Image* image = lctx.jobj_to_obj<Image>(holder_image);
		if (image != nullptr)
		{
			self->texture.load_memory_rgba(image->width(), image->height(), image->data(), true);
			self->repl_texture = nullptr;
		}
	}
	else if (clsname == "GLRenderTarget")
	{
		GLRenderTarget* target = lctx.jobj_to_obj<GLRenderTarget>(holder_image);
		if (target != nullptr)
		{
			self->repl_texture = target->m_tex_video.get();
		}
	}
#if THREE_MM
	else if (clsname == "MMCamera")
	{
		MMCamera* cam = lctx.jobj_to_obj<MMCamera>(holder_image);
		if (cam != nullptr)
		{
			self->repl_texture = cam->get_texture();
		}
	}
	else if (clsname == "MMLazyVideo")
	{
		MMLazyVideo* video = lctx.jobj_to_obj<MMLazyVideo>(holder_image);
		if (video != nullptr)
		{
			self->repl_texture = video->get_texture();
		}
	}
	else if (clsname == "MMVideo")
	{
		MMVideo* video = lctx.jobj_to_obj<MMVideo>(holder_image);
		if (video != nullptr)
		{
			self->repl_texture = video->get_texture();
		}
	}
#endif
}


void WrapperSimpleModel::GetMetalness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->material.metallicFactor));
}

void WrapperSimpleModel::SetMetalness(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	float metalness;
	lctx.jnum_to_num(value, metalness);
	self->set_metalness(metalness);
}


void WrapperSimpleModel::GetRoughness(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->material.roughnessFactor));
}

void WrapperSimpleModel::SetRoughness(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	float roughness;
	lctx.jnum_to_num(value, roughness);
	self->set_roughness(roughness);
}


void WrapperSimpleModel::SetToonShading(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	SimpleModel* self = lctx.self<SimpleModel>();
	int mode;
	lctx.jnum_to_num(info[0], mode);
	float width = 1.5f;
	glm::vec3 wire_color = { 0.0f, 0.0f, 0.0f };
	if (info.Length() > 1)
	{
		lctx.jnum_to_num(info[1], width);
		if (info.Length() > 2)
		{
			lctx.jvec3_to_vec3(info[2], wire_color);
		}
	}
	self->set_toon_shading(mode, width, wire_color);
}

