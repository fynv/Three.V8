#include "WrapperUtils.hpp"
#include <backgrounds/Background.h>
#include <utils/Image.h>

#include "WrapperBackground.h"

void WrapperColorBackground::define(ClassDefinition& cls)
{
	cls.name = "ColorBackground";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "color",  GetColor },		
	};
	cls.methods = {
		{"getColor", GetColor },
		{"setColor", SetColor },
	};
}

void* WrapperColorBackground::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new ColorBackground;
}

void WrapperColorBackground::dtor(void* ptr, GameContext* ctx)
{
	delete (ColorBackground*)ptr;
}

void WrapperColorBackground::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ColorBackground* self = lctx.self<ColorBackground>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->color, position);
	info.GetReturnValue().Set(position);
}

void WrapperColorBackground::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ColorBackground* self = lctx.self<ColorBackground>();
	lctx.vec3_to_jvec3(self->color, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperColorBackground::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ColorBackground* self = lctx.self<ColorBackground>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->color.x);
		lctx.jnum_to_num(info[1], self->color.y);
		lctx.jnum_to_num(info[2], self->color.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->color);
	}
}


void WrapperCubeBackground::define(ClassDefinition& cls)
{
	cls.name = "CubeBackground";
	cls.ctor = ctor;
	cls.dtor = dtor;	
	cls.methods = {
		{"setCubemap", SetCubemap },
	};
}

void* WrapperCubeBackground::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new CubeBackground;
}

void WrapperCubeBackground::dtor(void* ptr, GameContext* ctx)
{
	delete (CubeBackground*)ptr;
}

void WrapperCubeBackground::SetCubemap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeBackground* self = lctx.self<CubeBackground>();

	if (info[0]->IsNull())
	{
		self->cubemap.unload();
	}
	else
	{
		CubeImage* image = lctx.jobj_to_obj<CubeImage>(info[0]);
		self->cubemap.load_memory_rgba(image->images[0].width(), image->images[0].height(),
			image->images[0].data(), image->images[1].data(), image->images[2].data(), image->images[3].data(), image->images[4].data(), image->images[5].data());
	}
}


void WrapperPanoramaBackground::define(ClassDefinition& cls)
{
	cls.name = "PanoramaBackground";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.methods = {
		{"setTexture", SetTexture },
	};
}

void* WrapperPanoramaBackground::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new PanoramaBackground;
}

void WrapperPanoramaBackground::dtor(void* ptr, GameContext* ctx)
{
	delete (PanoramaBackground*)ptr;
}


void WrapperPanoramaBackground::SetTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	PanoramaBackground* self = lctx.self<PanoramaBackground>();

	if (info[0]->IsNull())
	{
		self->tex.unload();
	}
	else
	{
		Image* image = lctx.jobj_to_obj<Image>(info[0]);
		self->tex.load_memory_rgba(image->width(), image->height(), image->data(), true);
	}
}


void WrapperHemisphereBackground::define(ClassDefinition& cls)
{
	cls.name = "HemisphereBackground";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "skyColor",  GetSkyColor },
		{ "groundColor", GetGroundColor },
	};
	cls.methods = {
		{ "getSkyColor", GetSkyColor },
		{ "setSkyColor", SetSkyColor },
		{ "getGroundColor", GetGroundColor },
		{ "setGroundColor", SetGroundColor },
	};
}

void* WrapperHemisphereBackground::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new HemisphereBackground;
}

void WrapperHemisphereBackground::dtor(void* ptr, GameContext* ctx)
{
	delete (HemisphereBackground*)ptr;
}


void WrapperHemisphereBackground::GetSkyColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereBackground* self = new HemisphereBackground();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->skyColor, position);
	info.GetReturnValue().Set(position);
}

void WrapperHemisphereBackground::GetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereBackground* self = new HemisphereBackground();
	lctx.vec3_to_jvec3(self->skyColor, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperHemisphereBackground::SetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereBackground* self = lctx.self<HemisphereBackground>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->skyColor.x);
		lctx.jnum_to_num(info[1], self->skyColor.y);
		lctx.jnum_to_num(info[2], self->skyColor.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->skyColor);
	}
}


void WrapperHemisphereBackground::GetGroundColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereBackground* self = new HemisphereBackground();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->groundColor, position);
	info.GetReturnValue().Set(position);
}

void WrapperHemisphereBackground::GetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereBackground* self = new HemisphereBackground();
	lctx.vec3_to_jvec3(self->groundColor, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperHemisphereBackground::SetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereBackground* self = lctx.self<HemisphereBackground>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->groundColor.x);
		lctx.jnum_to_num(info[1], self->groundColor.y);
		lctx.jnum_to_num(info[2], self->groundColor.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->groundColor);
	}
}
