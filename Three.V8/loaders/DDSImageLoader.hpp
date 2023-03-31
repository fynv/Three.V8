#pragma once

#include "WrapperUtils.hpp"
#include <loaders/DDSImageLoader.h>

class WrapperDDSImageLoader
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info);	
};


v8::Local<v8::ObjectTemplate> WrapperDDSImageLoader::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "loadFile", v8::FunctionTemplate::New(isolate, LoadFile));
	templ->Set(isolate, "loadMemory", v8::FunctionTemplate::New(isolate, LoadMemory));	
	return templ;
}

void WrapperDDSImageLoader::LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	if (!exists_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Null(lctx.isolate));
		return;
	}
	v8::Local<v8::Object> holder = lctx.instantiate("DDSImage");
	DDSImage* self = lctx.jobj_to_obj<DDSImage>(holder);
	DDSImageLoader::LoadFile(self, filename.c_str());
	info.GetReturnValue().Set(holder);
}


void WrapperDDSImageLoader::LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("DDSImage");
	DDSImage* self = lctx.jobj_to_obj<DDSImage>(holder);
	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	DDSImageLoader::LoadMemory(self, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());
	info.GetReturnValue().Set(holder);
}

