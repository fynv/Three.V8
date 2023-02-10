#pragma once

#include "WrapperUtils.hpp"
#include <savers/FileSaver.h>

class WrapperFileSaver
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void SaveBinaryFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SaveTextFile(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::ObjectTemplate> WrapperFileSaver::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "saveBinaryFile", v8::FunctionTemplate::New(isolate, SaveBinaryFile));
	templ->Set(isolate, "saveTextFile", v8::FunctionTemplate::New(isolate, SaveTextFile));
	return templ;
}

void WrapperFileSaver::SaveBinaryFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	v8::Local<v8::ArrayBuffer> data = info[1].As<v8::ArrayBuffer>();
	const unsigned char* p_data = (const unsigned char* )data->GetBackingStore()->Data();
	size_t size = data->ByteLength();
	::SaveBinaryFile(filename.c_str(), p_data, size);
}

void WrapperFileSaver::SaveTextFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	std::string data = lctx.jstr_to_str(info[1]);
	::SaveTextFile(filename.c_str(), data.c_str());
}


