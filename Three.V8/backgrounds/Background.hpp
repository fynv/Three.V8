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
	static void Dispose(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetColor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperColorBackground::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(1);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, Dispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "color").ToLocalChecked(), GetColor, 0);
	templ->InstanceTemplate()->Set(isolate, "getColor", v8::FunctionTemplate::New(isolate, GetColor));
	templ->InstanceTemplate()->Set(isolate, "setColor", v8::FunctionTemplate::New(isolate, SetColor));

	return templ;
}

void WrapperColorBackground::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	ColorBackground* self = new ColorBackground();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));	
}

void WrapperColorBackground::Dispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	ColorBackground* self = get_self<ColorBackground>(info);
	delete self;
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
	static void Dispose(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void SetCubemap(const v8::FunctionCallbackInfo<v8::Value>& info);

};


v8::Local<v8::FunctionTemplate> WrapperCubeBackground::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(1);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, Dispose));
	templ->InstanceTemplate()->Set(isolate, "setCubemap", v8::FunctionTemplate::New(isolate, SetCubemap));
	return templ;
}

void WrapperCubeBackground::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	CubeBackground* self = new CubeBackground();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
}

void WrapperCubeBackground::Dispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	CubeBackground* self = get_self<CubeBackground>(info);
	delete self;
}

void WrapperCubeBackground::SetCubemap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	CubeBackground* self = get_self<CubeBackground>(info);
	Image* images[6];
	for (int i = 0; i < 6; i++)
	{
		v8::Local<v8::Object> holder_image = info[i].As<v8::Object>();
		images[i] = (Image*)v8::Local<v8::External>::Cast(holder_image->GetInternalField(0))->Value();
	}
	self->cubemap.load_memory_bgr(images[0]->width(), images[0]->height(), images[0]->data(), images[1]->data(), images[2]->data(), images[3]->data(), images[4]->data(), images[5]->data());
}

