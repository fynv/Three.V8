#pragma once

#include "WrapperUtils.hpp"
#include <savers/ImageSaver.h>

class WrapperImageSaver
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info);	
	static void SaveCubeToFile(const v8::FunctionCallbackInfo<v8::Value>& info);	
};


v8::Local<v8::ObjectTemplate> WrapperImageSaver::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "saveFile", v8::FunctionTemplate::New(isolate, SaveFile));	
	templ->Set(isolate, "saveCubeToFile", v8::FunctionTemplate::New(isolate, SaveCubeToFile));
	return templ;
}



void WrapperImageSaver::SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Object> holder = info[0].As<v8::Object>();
	Image* self = (Image*)holder->GetAlignedPointerFromInternalField(0);
	v8::String::Utf8Value filename(isolate, info[1]);
	ImageSaver::SaveFile(self, *filename);
}

void WrapperImageSaver::SaveCubeToFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Object> holder = info[0].As<v8::Object>();
	CubeImage* self = (CubeImage*)holder->GetAlignedPointerFromInternalField(0);

	v8::String::Utf8Value filenames[6] = {
		v8::String::Utf8Value(isolate, info[1]),
		v8::String::Utf8Value(isolate, info[2]),
		v8::String::Utf8Value(isolate, info[3]),
		v8::String::Utf8Value(isolate, info[4]),
		v8::String::Utf8Value(isolate, info[5]),
		v8::String::Utf8Value(isolate, info[6])
	};
	ImageSaver::SaveCubeToFile(self, *filenames[0], *filenames[1], *filenames[2], *filenames[3], *filenames[4], *filenames[5]);
}

