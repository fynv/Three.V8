#pragma once

#include "WrapperUtils.hpp"
#include <loaders/ImageLoader.h>

class WrapperImageLoader
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::ObjectTemplate> WrapperImageLoader::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "loadFile", v8::FunctionTemplate::New(isolate, LoadFile));
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
	v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(holder->GetInternalField(0));
	Image* self = (Image*)wrap->Value();
	
	v8::String::Utf8Value filename(isolate, info[0]);
	ImageLoader::LoadFile(self, *filename);
	info.GetReturnValue().Set(holder);
}
