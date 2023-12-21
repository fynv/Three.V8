#include "WrapperUtils.hpp"
#include <savers/ProbeGridSaver.h>
#include <utils/Utils.h>

#include "WrapperProbeGridSaver.h"

void WrapperProbeGridSaver::define(ObjectDefinition& object)
{
	object.name = "probeGridSaver";
	object.methods = {
		{ "saveFile", SaveFile},
	};
}

void WrapperProbeGridSaver::SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	ProbeGrid* self = lctx.jobj_to_obj<ProbeGrid>(info[0]);
	std::string filename = lctx.jstr_to_str(info[1]);

	if (!writable_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, false));
		return;
	}

	ProbeGridSaver::SaveFile(self, filename.c_str());

	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, true));
}


