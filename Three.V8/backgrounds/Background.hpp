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
	static void dtor(void* ptr);

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

void WrapperColorBackground::dtor(void* ptr)
{
	delete (ColorBackground*)ptr;
}


void WrapperColorBackground::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	ColorBackground* self = new ColorBackground();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));	
	info.This()->SetInternalField(1, v8::External::New(info.GetIsolate(), dtor));
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This());
}


void WrapperColorBackground::GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	ColorBackground* self = get_self<ColorBackground>(info);
	v8::Local<v8::Object> position = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->color, position);
	info.GetReturnValue().Set(position);
}

void WrapperColorBackground::GetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	ColorBackground* self = get_self<ColorBackground>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->color, out);
	info.GetReturnValue().Set(out);
}

void WrapperColorBackground::SetColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	ColorBackground* self = get_self<ColorBackground>(info);
	if (info[0]->IsNumber())
	{
		self->color.x = (float)info[0].As<v8::Number>()->Value();
		self->color.y = (float)info[1].As<v8::Number>()->Value();
		self->color.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, self->color);
	}
}


class WrapperCubeBackground
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr);

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


void WrapperCubeBackground::dtor(void* ptr)
{
	delete (CubeBackground*)ptr;
}


void WrapperCubeBackground::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	CubeBackground* self = new CubeBackground();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
	info.This()->SetInternalField(1, v8::External::New(info.GetIsolate(), dtor));
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This());
}


void WrapperCubeBackground::SetCubemap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	CubeBackground* self = get_self<CubeBackground>(info);
	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();
	CubeImage* image = (CubeImage*)v8::Local<v8::External>::Cast(holder_image->GetInternalField(0))->Value();
	self->cubemap.load_memory_bgr(image->images[0].width(), image->images[0].height(),
		image->images[0].data(), image->images[1].data(), image->images[2].data(), image->images[3].data(), image->images[4].data(), image->images[5].data());
}

class WrapperHemisphereBackground
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr);

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


void WrapperHemisphereBackground::dtor(void* ptr)
{
	delete (HemisphereBackground*)ptr;
}

void WrapperHemisphereBackground::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	HemisphereBackground* self = new HemisphereBackground();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
	info.This()->SetInternalField(1, v8::External::New(info.GetIsolate(), dtor));
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This());
}

void WrapperHemisphereBackground::GetSkyColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereBackground* self = get_self<HemisphereBackground>(info);
	v8::Local<v8::Object> position = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->skyColor, position);
	info.GetReturnValue().Set(position);
}

void WrapperHemisphereBackground::GetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereBackground* self = get_self<HemisphereBackground>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->skyColor, out);
	info.GetReturnValue().Set(out);
}

void WrapperHemisphereBackground::SetSkyColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereBackground* self = get_self<HemisphereBackground>(info);
	if (info[0]->IsNumber())
	{
		self->skyColor.x = (float)info[0].As<v8::Number>()->Value();
		self->skyColor.y = (float)info[1].As<v8::Number>()->Value();
		self->skyColor.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, self->skyColor);
	}
}


void WrapperHemisphereBackground::GetGroundColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereBackground* self = get_self<HemisphereBackground>(info);
	v8::Local<v8::Object> position = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->groundColor, position);
	info.GetReturnValue().Set(position);
}

void WrapperHemisphereBackground::GetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereBackground* self = get_self<HemisphereBackground>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->groundColor, out);
	info.GetReturnValue().Set(out);
}

void WrapperHemisphereBackground::SetGroundColor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	HemisphereBackground* self = get_self<HemisphereBackground>(info);
	if (info[0]->IsNumber())
	{
		self->groundColor.x = (float)info[0].As<v8::Number>()->Value();
		self->groundColor.y = (float)info[1].As<v8::Number>()->Value();
		self->groundColor.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, self->groundColor);
	}
}
