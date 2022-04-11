#pragma once

#include "WrapperUtils.hpp"
#include <loaders/FileLoader.h>

class WrapperFileLoader
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void LoadBinaryFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadTextFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	
};

v8::Local<v8::ObjectTemplate> WrapperFileLoader::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "loadBinaryFile", v8::FunctionTemplate::New(isolate, LoadBinaryFile));
	templ->Set(isolate, "loadTextFile", v8::FunctionTemplate::New(isolate, LoadTextFile));
	return templ;
}


void WrapperFileLoader::LoadBinaryFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8::String::Utf8Value filename(isolate, info[0]);

	std::vector<unsigned char> data;
	::LoadBinaryFile(*filename, data);

	v8::Local<v8::ArrayBuffer> ret = v8::ArrayBuffer::New(isolate, data.size());
	void* p_data = ret->GetBackingStore()->Data();
	memcpy(p_data, data.data(), data.size());
	
	info.GetReturnValue().Set(ret);
}

void WrapperFileLoader::LoadTextFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8::String::Utf8Value filename(isolate, info[0]);

	std::vector<char> data;
	::LoadTextFile(*filename, data);

	v8::Local<v8::String> ret = v8::String::NewFromUtf8(isolate, data.data()).ToLocalChecked();
	info.GetReturnValue().Set(ret);
}
