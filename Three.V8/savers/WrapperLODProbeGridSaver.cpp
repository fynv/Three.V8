#include "WrapperUtils.hpp"
#include <savers/LODProbeGridSaver.h>
#include <utils/Utils.h>

#include "WrapperLODProbeGridSaver.h"

void WrapperLODProbeGridSaver::define(ObjectDefinition& object)
{
	object.name = "LODProbeGridSaver";
	object.methods = {
		{ "saveFile", SaveFile},
	};
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

