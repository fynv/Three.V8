#pragma once

#include "WrapperUtils.hpp"
#include <savers/ImageSaver.h>
#include <utils/Utils.h>

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
	LocalContext lctx(info);
	Image* self = lctx.jobj_to_obj<Image>(info[0]);
	std::string filename = lctx.jstr_to_str(info[1]);

	if (!writable_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, false));
		return;
	}

	ImageSaver::SaveFile(self, filename.c_str());

	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, true));
}

void WrapperImageSaver::SaveCubeToFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	CubeImage* self = lctx.jobj_to_obj<CubeImage>(info[0]);	

	std::string filenames[6] = {
		lctx.jstr_to_str(info[1]),
		lctx.jstr_to_str(info[2]),
		lctx.jstr_to_str(info[3]),
		lctx.jstr_to_str(info[4]),
		lctx.jstr_to_str(info[5]),
		lctx.jstr_to_str(info[6])
	};
	
	for (int i = 0; i < 6; i++)
	{
		if (!writable_test(filenames[i].c_str()))
		{
			info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, false));
			return;
		}
	}

	ImageSaver::SaveCubeToFile(self, filenames[0].c_str(), filenames[1].c_str(), 
		filenames[2].c_str(), filenames[3].c_str(), filenames[4].c_str(), filenames[5].c_str());

	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, true));
}

