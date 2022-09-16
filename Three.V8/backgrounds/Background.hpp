#pragma once

#include "WrapperUtils.hpp"
#include <backgrounds/Background.h>
#include <utils/Image.h>

class WrapperColorBackground
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);

	static void GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperColorBackground::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "color").ToLocalChecked(), GetColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getColor", v8::FunctionTemplate::New(isolate, GetColor));
	templ->InstanceTemplate()->Set(isolate, "setColor", v8::FunctionTemplate::New(isolate, SetColor));

	return templ;
}

void WrapperColorBackground::dtor(void* ptr, GameContext* ctx)
{
	delete (ColorBackground*)ptr;
}


void WrapperColorBackground::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ColorBackground* self = new ColorBackground();
	info.This()->SetAlignedPointerInInternalField(0, self);		
	lctx.ctx()->regiter_object(info.This(), dtor);
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


class WrapperCubeBackground
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);

	static void SetCubemap(const v8::FunctionCallbackInfo<v8::Value>& info);

};


v8::Local<v8::FunctionTemplate> WrapperCubeBackground::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "setCubemap", v8::FunctionTemplate::New(isolate, SetCubemap));
	return templ;
}


void WrapperCubeBackground::dtor(void* ptr, GameContext* ctx)
{
	delete (CubeBackground*)ptr;
}


void WrapperCubeBackground::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeBackground* self = new CubeBackground();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
}


void WrapperCubeBackground::SetCubemap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeBackground* self = lctx.self<CubeBackground>();
	CubeImage* image = lctx.jobj_to_obj<CubeImage>(info[0]);
	self->cubemap.load_memory_rgba(image->images[0].width(), image->images[0].height(),
		image->images[0].data(), image->images[1].data(), image->images[2].data(), image->images[3].data(), image->images[4].data(), image->images[5].data());
}

class WrapperHemisphereBackground
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr, GameContext* ctx);

	static void GetSkyColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetGroundColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperHemisphereBackground::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "skyColor").ToLocalChecked(), GetSkyColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getSkyColor", v8::FunctionTemplate::New(isolate, GetSkyColor));
	templ->InstanceTemplate()->Set(isolate, "setSkyColor", v8::FunctionTemplate::New(isolate, SetSkyColor));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "groundColor").ToLocalChecked(), GetGroundColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getGroundColor", v8::FunctionTemplate::New(isolate, GetGroundColor));
	templ->InstanceTemplate()->Set(isolate, "setGroundColor", v8::FunctionTemplate::New(isolate, SetGroundColor));

	return templ;
}


void WrapperHemisphereBackground::dtor(void* ptr, GameContext* ctx)
{
	delete (HemisphereBackground*)ptr;
}

void WrapperHemisphereBackground::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HemisphereBackground* self = new HemisphereBackground();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
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
