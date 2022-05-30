#pragma once

#include "WrapperUtils.hpp"
#include <loaders/ImageLoader.h>

class WrapperImageLoader
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadCubeFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadCubeFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::ObjectTemplate> WrapperImageLoader::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "loadFile", v8::FunctionTemplate::New(isolate, LoadFile));
	templ->Set(isolate, "loadMemory", v8::FunctionTemplate::New(isolate, LoadMemory));
	templ->Set(isolate, "loadCubeFromFile", v8::FunctionTemplate::New(isolate, LoadCubeFromFile));
	templ->Set(isolate, "loadCubeFromMemory", v8::FunctionTemplate::New(isolate, LoadCubeFromMemory));
	return templ;
}

void WrapperImageLoader::LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Function> ctor_image = global->Get(context, v8::String::NewFromUtf8(isolate, "Image").ToLocalChecked()).ToLocalChecked().As<v8::Function>();
	
	v8::Local<v8::Object> holder = ctor_image->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	Image* self = (Image*)holder->GetAlignedPointerFromInternalField(0);
	
	v8::String::Utf8Value filename(isolate, info[0]);
	ImageLoader::LoadFile(self, *filename);
	info.GetReturnValue().Set(holder);
}


void WrapperImageLoader::LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Function> ctor_image = global->Get(context, v8::String::NewFromUtf8(isolate, "Image").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

	v8::Local<v8::Object> holder = ctor_image->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	Image* self = (Image*)holder->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	ImageLoader::LoadMemory(self, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());	
	info.GetReturnValue().Set(holder);
}

void WrapperImageLoader::LoadCubeFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Function> ctor_image = global->Get(context, v8::String::NewFromUtf8(isolate, "CubeImage").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

	v8::Local<v8::Object> holder = ctor_image->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	CubeImage* self = (CubeImage*)holder->GetAlignedPointerFromInternalField(0);

	v8::String::Utf8Value filenames[6] = {
		v8::String::Utf8Value(isolate, info[0]),
		v8::String::Utf8Value(isolate, info[1]),
		v8::String::Utf8Value(isolate, info[2]),
		v8::String::Utf8Value(isolate, info[3]),
		v8::String::Utf8Value(isolate, info[4]),
		v8::String::Utf8Value(isolate, info[5])
	};
	
	ImageLoader::LoadCubeFromFile(self, *filenames[0], *filenames[1], *filenames[2], *filenames[3], *filenames[4], *filenames[5]);
	info.GetReturnValue().Set(holder);
}


void WrapperImageLoader::LoadCubeFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Function> ctor_image = global->Get(context, v8::String::NewFromUtf8(isolate, "CubeImage").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

	v8::Local<v8::Object> holder = ctor_image->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	CubeImage* self = (CubeImage*)holder->GetAlignedPointerFromInternalField(0);

	v8::Local<v8::ArrayBuffer> data[6];
	for (int i = 0; i < 6; i++)
	{
		data [i] = info[i].As<v8::ArrayBuffer>();
	}
	ImageLoader::LoadCubeFromMemory(self, 
		(unsigned char*)data[0]->GetBackingStore()->Data(), data[0]->ByteLength(),
		(unsigned char*)data[1]->GetBackingStore()->Data(), data[1]->ByteLength(),
		(unsigned char*)data[2]->GetBackingStore()->Data(), data[2]->ByteLength(),
		(unsigned char*)data[3]->GetBackingStore()->Data(), data[3]->ByteLength(), 
		(unsigned char*)data[4]->GetBackingStore()->Data(), data[4]->ByteLength(),
		(unsigned char*)data[5]->GetBackingStore()->Data(), data[5]->ByteLength());

	info.GetReturnValue().Set(holder);
}
