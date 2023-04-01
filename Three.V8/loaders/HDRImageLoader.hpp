#pragma once

#include "WrapperUtils.hpp"
#include <loaders/HDRImageLoader.h>

class WrapperHDRImageLoader
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadCubeFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadCubeFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void FromImages(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::ObjectTemplate> WrapperHDRImageLoader::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "loadFile", v8::FunctionTemplate::New(isolate, LoadFile));
	templ->Set(isolate, "loadMemory", v8::FunctionTemplate::New(isolate, LoadMemory));
	templ->Set(isolate, "loadCubeFromFile", v8::FunctionTemplate::New(isolate, LoadCubeFromFile));
	templ->Set(isolate, "loadCubeFromMemory", v8::FunctionTemplate::New(isolate, LoadCubeFromMemory));
	templ->Set(isolate, "fromImages", v8::FunctionTemplate::New(isolate, FromImages));
	return templ;
}

void WrapperHDRImageLoader::LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	if (!exists_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Null(lctx.isolate));
		return;
	}
	v8::Local<v8::Object> holder = lctx.instantiate("HDRImage");
	HDRImage* self = lctx.jobj_to_obj<HDRImage>(holder);
	HDRImageLoader::LoadFile(self, filename.c_str());
	info.GetReturnValue().Set(holder);
}


void WrapperHDRImageLoader::LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("HDRImage");
	HDRImage* self = lctx.jobj_to_obj<HDRImage>(holder);
	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	HDRImageLoader::LoadMemory(self, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());
	info.GetReturnValue().Set(holder);
}

void WrapperHDRImageLoader::LoadCubeFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);	
	std::string filenames[6] =
	{
		lctx.jstr_to_str(info[0]),
		lctx.jstr_to_str(info[1]),
		lctx.jstr_to_str(info[2]),
		lctx.jstr_to_str(info[3]),
		lctx.jstr_to_str(info[4]),
		lctx.jstr_to_str(info[5])
	};

	for (int i = 0; i < 6; i++)
	{
		if (!exists_test(filenames[i].c_str()))
		{
			info.GetReturnValue().Set(v8::Null(lctx.isolate));
			return;
		}
	}
	
	v8::Local<v8::Object> holder = lctx.instantiate("HDRCubeImage");
	HDRCubeImage* self = lctx.jobj_to_obj<HDRCubeImage>(holder);
	HDRImageLoader::LoadCubeFromFile(self, filenames[0].c_str(), filenames[1].c_str(), 
		filenames[2].c_str(), filenames[3].c_str(), filenames[4].c_str(), filenames[5].c_str());

	info.GetReturnValue().Set(holder);
}


void WrapperHDRImageLoader::LoadCubeFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("HDRCubeImage");
	HDRCubeImage* self = lctx.jobj_to_obj<HDRCubeImage>(holder);

	v8::Local<v8::ArrayBuffer> data[6];
	for (int i = 0; i < 6; i++)
	{
		data[i] = info[i].As<v8::ArrayBuffer>();
	}
	HDRImageLoader::LoadCubeFromMemory(self,
		(unsigned char*)data[0]->GetBackingStore()->Data(), data[0]->ByteLength(),
		(unsigned char*)data[1]->GetBackingStore()->Data(), data[1]->ByteLength(),
		(unsigned char*)data[2]->GetBackingStore()->Data(), data[2]->ByteLength(),
		(unsigned char*)data[3]->GetBackingStore()->Data(), data[3]->ByteLength(), 
		(unsigned char*)data[4]->GetBackingStore()->Data(), data[4]->ByteLength(),
		(unsigned char*)data[5]->GetBackingStore()->Data(), data[5]->ByteLength());

	info.GetReturnValue().Set(holder);
}

void WrapperHDRImageLoader::FromImages(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	v8::Local<v8::Array> lst_images = info[0].As<v8::Array>();
	v8::Local<v8::Array> lst_ranges = info[1].As<v8::Array>();

	int count = (int)lst_images->Length();
	std::vector<const Image*> images(count);
	std::vector<HDRImageLoader::Range> ranges(count);

	for (int i = 0; i < count; i++)
	{
		Image* img = lctx.jobj_to_obj<Image>(lst_images->Get(lctx.context, i).ToLocalChecked());
		images[i] = img;
		v8::Local<v8::Object> j_range = lst_ranges->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		v8::Local<v8::Value> j_low = lctx.get_property(j_range, "low");
		v8::Local<v8::Value> j_high = lctx.get_property(j_range, "high");
		HDRImageLoader::Range range;
		lctx.jvec3_to_vec3(j_low, range.low);
		lctx.jvec3_to_vec3(j_high, range.high);		
		ranges[i] = range;
	}

	v8::Local<v8::Object> holder = lctx.instantiate("HDRImage");
	HDRImage* self = lctx.jobj_to_obj<HDRImage>(holder);	
	HDRImageLoader::FromImages(self, images.data(), ranges.data(), count);	
	info.GetReturnValue().Set(holder);
}

