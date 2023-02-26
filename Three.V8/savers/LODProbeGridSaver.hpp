#pragma once

#include "WrapperUtils.hpp"
#include <savers/LODProbeGridSaver.h>
#include <utils/Utils.h>

class WrapperLODProbeGridSaver
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info);
};



v8::Local<v8::ObjectTemplate> WrapperLODProbeGridSaver::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "saveFile", v8::FunctionTemplate::New(isolate, SaveFile));
	return templ;
}

void WrapperLODProbeGridSaver::SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	LODProbeGrid* self = lctx.jobj_to_obj<LODProbeGrid>(info[0]);
	std::string filename = lctx.jstr_to_str(info[1]);

	if (!writable_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, false));
		return;
	}

	LODProbeGridSaver::SaveFile(self, filename.c_str());

	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, true));
}

