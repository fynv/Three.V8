#pragma once

#include "WrapperUtils.hpp"
#include <savers/ProbeGridSaver.h>

class WrapperProbeGridSaver
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info);	
};


v8::Local<v8::ObjectTemplate> WrapperProbeGridSaver::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "saveFile", v8::FunctionTemplate::New(isolate, SaveFile));	
	return templ;
}

void WrapperProbeGridSaver::SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.jobj_to_obj<ProbeGrid>(info[0]);
	std::string filename = lctx.jstr_to_str(info[1]);
	ProbeGridSaver::SaveFile(self, filename.c_str());
}

