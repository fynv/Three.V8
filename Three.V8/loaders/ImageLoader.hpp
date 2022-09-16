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
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("Image");
	Image* self = lctx.jobj_to_obj<Image>(holder);	
	std::string filename = lctx.jstr_to_str(info[0]);
	ImageLoader::LoadFile(self, filename.c_str());
	info.GetReturnValue().Set(holder);
}


void WrapperImageLoader::LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("Image");
	Image* self = lctx.jobj_to_obj<Image>(holder);
	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	ImageLoader::LoadMemory(self, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());	
	info.GetReturnValue().Set(holder);
}

void WrapperImageLoader::LoadCubeFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("CubeImage");
	CubeImage* self = lctx.jobj_to_obj<CubeImage>(holder);
	std::string filenames[6] =
	{
		lctx.jstr_to_str(info[0]),
		lctx.jstr_to_str(info[1]),
		lctx.jstr_to_str(info[2]),
		lctx.jstr_to_str(info[3]),
		lctx.jstr_to_str(info[4]),
		lctx.jstr_to_str(info[5])
	};
	
	ImageLoader::LoadCubeFromFile(self, filenames[0].c_str(), filenames[1].c_str(), 
		filenames[2].c_str(), filenames[3].c_str(), filenames[4].c_str(), filenames[5].c_str());

	info.GetReturnValue().Set(holder);
}


void WrapperImageLoader::LoadCubeFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("CubeImage");
	CubeImage* self = lctx.jobj_to_obj<CubeImage>(holder);

	v8::Local<v8::ArrayBuffer> data[6];
	for (int i = 0; i < 6; i++)
	{
		data[i] = info[i].As<v8::ArrayBuffer>();
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
